#include "lvox_steploadinfile.h"

#include "ct_view/ct_stepconfigurabledialog.h"
#include "ct_result/model/inModel/ct_inresultmodelgroup.h"
#include "ct_result/model/outModel/ct_outresultmodelgroup.h"

// Inclusion of standard result class
#include "ct_result/ct_resultgroup.h"

// Inclusion of used ItemDrawable classes
#include "ct_itemdrawable/ct_scanner.h"
#include "ct_itemdrawable/ct_scene.h"
#include "ct_itemdrawable/ct_pointsattributescolor.h"
#include "ct_itemdrawable/ct_pointsattributesscalartemplated.h"
#include "ct_global/ct_context.h"
#include "ct_model/tools/ct_modelsearchhelper.h"

#include <limits>

#include "ct_reader/ct_reader_ascrgb.h"
#include "ct_reader/ct_reader_xyb.h"

#include "qfile.h"
#include "qtextstream.h"
#include "qfileinfo.h"
#include "qdebug.h"

// Alias for indexing out models
#define DEF_resultOut_individualScenes "rsce"
#define DEF_groupOut_g "g"
#define DEF_itemOut_individualScene "sce"
#define DEF_itemOut_individualSceneColor "col"
#define DEF_itemOut_individualSceneIntensity "int"
#define DEF_itemOut_scanner "sca"
#define DEF_resultOut_mergedScene "rmsce"
#define DEF_groupOut_gm "gm"
#define DEF_itemOut_mergedScene "scm"

// Constructor : initialization of parameters
LVOX_StepLoadInFile::LVOX_StepLoadInFile(CT_StepInitializeData &dataInit) : CT_AbstractStepCanBeAddedFirst(dataInit)
{
    _radiusFiltered = true;
    _radius = 20;
}

// Step description (tooltip of contextual menu)
QString LVOX_StepLoadInFile::getStepDescription() const
{
    return tr("1- Charger un fichier .in LVOX");
}

// Step copy method
CT_VirtualAbstractStep* LVOX_StepLoadInFile::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new LVOX_StepLoadInFile(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void LVOX_StepLoadInFile::createInResultModelListProtected()
{
    setNotNeedInputResult();
}

// Creation and affiliation of OUT models
void LVOX_StepLoadInFile::createOutResultModelListProtected()
{
    CT_OutResultModelGroup *resultModel = createNewOutResultModel(DEF_resultOut_individualScenes, tr("IndividualScenes"));

    resultModel->setRootGroup(DEF_groupOut_g);
    resultModel->addItemModel(DEF_groupOut_g, DEF_itemOut_individualScene, new CT_Scene(), tr("IndividualScene"));
    resultModel->addItemModel(DEF_groupOut_g, DEF_itemOut_individualSceneColor, new CT_PointsAttributesColor(), tr("IndividualSceneColors"));
    resultModel->addItemModel(DEF_groupOut_g, DEF_itemOut_individualSceneIntensity, new CT_PointsAttributesScalarTemplated<quint16>(), tr("IndividualSceneIntensity"));
    resultModel->addItemModel(DEF_groupOut_g, DEF_itemOut_scanner, new CT_Scanner(), tr("ScanPosition"));


    CT_OutResultModelGroup *resultMergedModel = createNewOutResultModel(DEF_resultOut_mergedScene, tr("MergedScene"));

    resultMergedModel->setRootGroup(DEF_groupOut_gm);
    resultMergedModel->addItemModel(DEF_groupOut_gm, DEF_itemOut_mergedScene, new CT_Scene(), tr("MergedScene"));
}

// Semi-automatic creation of step parameters DialogBox
void LVOX_StepLoadInFile::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addFileChoice(tr("Choisir le fichier .in"), CT_FileChoiceButton::OneExistingFile, "Fichier in (*.in)", _fileName);
    configDialog->addBool(tr("Extraire une placette de rayon fixé"), "", "", _radiusFiltered);
    configDialog->addDouble(tr("Rayon de la placette"), "m", 0.1, 1000, 2, _radius, 0);
}

void LVOX_StepLoadInFile::compute()
{
    QList<CT_ResultGroup*> outResultList = getOutResultList();

    CT_ResultGroup* resultOut_individualScenes = outResultList.at(0);
    CT_OutAbstractSingularItemModel *itemOutModel_individualScene = (CT_OutAbstractSingularItemModel*)PS_MODELS->searchModel(DEF_itemOut_individualScene, resultOut_individualScenes, this);
    CT_OutAbstractSingularItemModel *itemOutModel_individualSceneColor = (CT_OutAbstractSingularItemModel*)PS_MODELS->searchModel(DEF_itemOut_individualSceneColor, resultOut_individualScenes, this);
    CT_OutAbstractSingularItemModel *itemOutModel_individualSceneIntensity = (CT_OutAbstractSingularItemModel*)PS_MODELS->searchModel(DEF_itemOut_individualSceneIntensity, resultOut_individualScenes, this);

    CT_ResultGroup* resultOut_mergedScene = outResultList.at(1);

    if (!_radiusFiltered) {_radius = 0;}

    QMap<QString, CT_Scanner*> scansMap = readInFile(_fileName.first(), resultOut_individualScenes);

    if (scansMap.size()<=0) {return;}

    double xmin = std::numeric_limits<double>::max();
    double ymin = std::numeric_limits<double>::max();
    double zmin = std::numeric_limits<double>::max();

    double xmax = -std::numeric_limits<double>::max();
    double ymax = -std::numeric_limits<double>::max();
    double zmax = -std::numeric_limits<double>::max();

    QList<CT_PCIR> individualScenes;

    int baseProgress = 0;
    int progressIncrement = 100/scansMap.size();

    QMapIterator<QString, CT_Scanner*> it (scansMap);

    while (it.hasNext())
    {
        it.next();

        QFileInfo fileInfo(it.key());

        if (fileInfo.exists())
        {
            CT_Scanner* scanner = it.value();

            QString extension = fileInfo.completeSuffix();
            CT_AbstractReader *reader;

            if (extension == "asc" || extension == "xyz" || extension == "xyz.asc")
            {
                reader = new CT_Reader_ASCRGB();
                PS_LOG->addMessage(LogInterface::trace, LogInterface::step, QObject::tr("File reader created"));
                if (_radius >0) {((CT_Reader_ASCRGB*) reader)->setRadiusFilter(_radius);}
            } else if (extension == "xyb")
            {
                reader = new CT_Reader_XYB();
                if (_radius >0) {((CT_Reader_XYB*) reader)->setRadiusFilter(_radius);}
            } else
            {
                PS_LOG->addMessage(LogInterface::trace, LogInterface::step, QObject::tr("Wrong file extension"));
                break;
            }


            if (reader->setFilePath(it.key()))
            {

                reader->init();

                if (reader->readFile())
                {
                    CT_Scene* scene = (CT_Scene*) reader->takeFirstItemDrawableOfType(CT_Scene::staticGetType(), resultOut_individualScenes, itemOutModel_individualScene);
                    // Colors are only obtained if ASCRGB
                    CT_PointsAttributesColor* colors = (CT_PointsAttributesColor*) reader->takeFirstItemDrawableOfType(CT_PointsAttributesColor::staticGetType(), resultOut_individualScenes, itemOutModel_individualSceneColor);
                    // Intensity is only obtained if XYB
                    CT_PointsAttributesScalarTemplated<quint16>* intensity = (CT_PointsAttributesScalarTemplated<quint16>*) reader->takeFirstItemDrawableOfType(CT_PointsAttributesScalarTemplated<quint16>::staticGetType(), resultOut_individualScenes, itemOutModel_individualSceneIntensity);

                    if (scene != NULL)
                    {

                        CT_StandardItemGroup* groupOut_individualScene = new CT_StandardItemGroup(DEF_groupOut_g, resultOut_individualScenes);

                        individualScenes.append(scene->getPointCloudIndexRegistered());

                        Eigen::Vector3d min, max;
                        scene->getBoundingBox(min, max);
                        if (min(0) < xmin) {xmin = min(0);}
                        if (max(0) > xmax) {xmax = max(0);}
                        if (min(1) < ymin) {ymin = min(1);}
                        if (max(1) > ymax) {ymax = max(1);}
                        if (min(2) < zmin) {zmin = min(2);}
                        if (max(2) > zmax) {zmax = max(2);}

                        groupOut_individualScene->addItemDrawable(scene);

                        if (scanner != NULL) {groupOut_individualScene->addItemDrawable(scanner);}


                        if (colors != NULL) {groupOut_individualScene->addItemDrawable(colors);}
                        if (intensity != NULL) {groupOut_individualScene->addItemDrawable(intensity);}

                        resultOut_individualScenes->addGroup(groupOut_individualScene);
                    }

                } else {delete scanner;}
            } else {delete scanner;}

            delete reader;
        }

        baseProgress += progressIncrement;
        setProgress(baseProgress);
    }


    CT_StandardItemGroup* groupOut_mergedScene = new CT_StandardItemGroup(DEF_groupOut_gm, resultOut_mergedScene);

    CT_Scene *mergedScene = new CT_Scene(DEF_itemOut_mergedScene, resultOut_mergedScene, PS_REPOSITORY->mergePointCloudContiguous(individualScenes));
    mergedScene->setBoundingBox(xmin,ymin,zmin,xmax,ymax,zmax);

    groupOut_mergedScene->addItemDrawable(mergedScene);
    resultOut_mergedScene->addGroup(groupOut_mergedScene);

    setProgress(100);
}

QMap<QString, CT_Scanner*> LVOX_StepLoadInFile::readInFile(QString filename, const CT_AbstractResult *result)
{
    QMap<QString, CT_Scanner*> map;

    if(QFile::exists(filename))
    {
        QFile f(filename);
        QString path = QFileInfo(filename).path();

        if (f.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream stream(&f);

            QString line = stream.readLine();
            while(line != NULL)
            {
                QStringList values = line.split(" ", QString::SkipEmptyParts);

                if (values.size() >= 10)
                {
                    QString scanFileName = values.at(0);
                    QString scannerModel = values.at(1);
                    bool  clockWise = (scannerModel=="FARO" || scannerModel=="Z+F");

                    bool okX = false;
                    bool okY = false;
                    bool okZ = false;
                    bool okRes = false;
                    bool okPhiDeb = false;
                    bool okPhiFin = false;
                    bool okThetaDeb = false;
                    bool okThetaFin = false;

                    double x = values.at(2).toDouble(&okX);
                    double y = values.at(3).toDouble(&okY);
                    double z = values.at(4).toDouble(&okZ);
                    double res = values.at(5).toDouble(&okRes);
                    double thetaDeb = values.at(6).toDouble(&okThetaDeb);
                    double thetaFin = values.at(7).toDouble(&okThetaFin);
                    double phiDeb = values.at(8).toDouble(&okPhiDeb);
                    double phiFin = values.at(9).toDouble(&okPhiFin);

                    if (okX && okY && okZ && okRes && okPhiDeb && okPhiFin && okThetaDeb && okThetaFin)
                    {
                        int scanID =0;
                        double hFov = (thetaFin - thetaDeb)*M_PI/180;
                        double vFov = (phiFin - phiDeb)*M_PI/180;
                        double initPhi = phiDeb*M_PI/180;
                        double initTheta = thetaDeb*M_PI/180;
                        double resolution = res*M_PI/180;

                        QString completeFileName = QString("%1/%2").arg(path).arg(scanFileName);
                        if(QFile::exists(completeFileName))
                        {
                            CT_Scanner *scanner = new CT_Scanner(DEF_itemOut_scanner, result, scanID++, Eigen::Vector3d(x, y, z),Eigen::Vector3d(0,0,1), hFov, vFov, resolution, resolution, initTheta, initPhi,  clockWise, true);
                            map.insert(completeFileName, scanner);
                        }
                    }
                }
                line = stream.readLine();
            }
            f.close();
        }
    }

    return map;
}
