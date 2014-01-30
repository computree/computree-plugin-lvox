#include "lvox_steploadinfile.h"

// Inclusion of in models
#include "ct_result/model/inModel/ct_inresultmodelnotneedinputresult.h"

// Inclusion of out models
#include "ct_itemdrawable/model/outModel/ct_outstandardgroupmodel.h"
#include "ct_itemdrawable/model/outModel/ct_outstandardgroupmodel.h"
#include "ct_itemdrawable/model/outModel/ct_outstandarditemdrawablemodel.h"
#include "ct_result/model/outModel/ct_outresultmodelgroup.h"

// Inclusion of standard result class
#include "ct_result/ct_resultgroup.h"

// Inclusion of used ItemDrawable classes
#include "ct_itemdrawable/ct_scanner.h"
#include "ct_itemdrawable/ct_scene.h"
#include "ct_itemdrawable/ct_pointsattributescolor.h"
#include "ct_global/ct_context.h"

#include <limits>

#include "ct_reader/ct_reader_ascrgb.h"
#include "qfile.h"
#include "qtextstream.h"
#include "qfileinfo.h"
#include "qdebug.h"

// Alias for indexing out models
#define DEF_resultOut_individualScenes "rsce"
#define DEF_groupOut_g "g"
#define DEF_itemOut_individualScene "sce"
#define DEF_itemOut_individualSceneColor "col"
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
    return "Load multi-scans for a plot of given radius, specified in a .in file";
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
    CT_InResultModelNotNeedInputResult *resultModel = new CT_InResultModelNotNeedInputResult();
    addInResultModel(resultModel);
}

// Creation and affiliation of OUT models
void LVOX_StepLoadInFile::createOutResultModelListProtected()
{

    CT_OutStandardGroupModel *groupOutModel_individualScenes = new CT_OutStandardGroupModel(DEF_groupOut_g,
                                                                             new CT_StandardItemGroup(), 
                                                                             tr("g"));

    CT_OutStandardItemDrawableModel *itemOutModel_individualScene = new CT_OutStandardItemDrawableModel(DEF_itemOut_individualScene,
                                                                                            new CT_Scene(), 
                                                                                            tr("IndividualScene"));

    CT_OutStandardItemDrawableModel *itemOutModel_individualSceneColor = new CT_OutStandardItemDrawableModel(DEF_itemOut_individualSceneColor,
                                                                                            new CT_PointsAttributesColor(),
                                                                                            tr("IndividualSceneColors"));

    CT_OutStandardItemDrawableModel *itemOutModel_scanner = new CT_OutStandardItemDrawableModel(DEF_itemOut_scanner, 
                                                                                                new CT_Scanner(), 
                                                                                                tr("ScanPosition"));

    groupOutModel_individualScenes->addItem(itemOutModel_individualScene);
    groupOutModel_individualScenes->addItem(itemOutModel_individualSceneColor);
    groupOutModel_individualScenes->addItem(itemOutModel_scanner);

    CT_OutResultModelGroup *resultOut_individualScenes = new CT_OutResultModelGroup(DEF_resultOut_individualScenes,
                                                                                  groupOutModel_individualScenes,
                                                                                  tr("IndividualScenes"));
    addOutResultModel(resultOut_individualScenes);

    CT_OutStandardGroupModel *groupOutModel_mergedScene = new CT_OutStandardGroupModel(DEF_groupOut_gm,
                                                                              new CT_StandardItemGroup(), 
                                                                              tr("gm"));

    CT_OutStandardItemDrawableModel *itemOutModel_mergedScene = new CT_OutStandardItemDrawableModel(DEF_itemOut_mergedScene,
                                                                                            new CT_Scene(), 
                                                                                            tr("MergedScene"));

    groupOutModel_mergedScene->addItem(itemOutModel_mergedScene);

    CT_OutResultModelGroup *resultOutModel_mergedScene = new CT_OutResultModelGroup(DEF_resultOut_mergedScene,
                                                                               groupOutModel_mergedScene,
                                                                               tr("MergedScene"));
    addOutResultModel(resultOutModel_mergedScene);
}

// Semi-automatic creation of step parameters DialogBox
void LVOX_StepLoadInFile::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addFileChoice("Choisir le fichier .in", CT_FileChoiceButton::OneExistingFile, "Fichier in (*.in)", _fileName);
    configDialog->addBool("Extraire une placette de rayon fixé", "", "", _radiusFiltered);
    configDialog->addDouble("Rayon de la placette", "m", 0.1, 1000, 2, _radius, 0);
}

void LVOX_StepLoadInFile::compute()
{
    QList<CT_ResultGroup*> outResultList = getOutResultList();

    CT_ResultGroup* resultOut_individualScenes = outResultList.at(0);
    CT_OutStandardGroupModel* groupOutModel_individualScenes = (CT_OutStandardGroupModel*)getOutModelForCreation(resultOut_individualScenes, DEF_groupOut_g);
    CT_OutStandardItemDrawableModel* itemOutModel_individualScene = (CT_OutStandardItemDrawableModel*)getOutModelForCreation(resultOut_individualScenes, DEF_itemOut_individualScene);
    CT_OutStandardItemDrawableModel* itemOutModel_individualSceneColor = (CT_OutStandardItemDrawableModel*)getOutModelForCreation(resultOut_individualScenes, DEF_itemOut_individualSceneColor);
    CT_OutStandardItemDrawableModel* itemOutModel_scanner = (CT_OutStandardItemDrawableModel*)getOutModelForCreation(resultOut_individualScenes, DEF_itemOut_scanner);

    CT_ResultGroup* resultOut_mergedScene = outResultList.at(1);
    CT_OutStandardGroupModel* groupOutModel_mergedScene = (CT_OutStandardGroupModel*)getOutModelForCreation(resultOut_mergedScene, DEF_groupOut_gm);
    CT_OutStandardItemDrawableModel* itemOutModel_mergedScene = (CT_OutStandardItemDrawableModel*)getOutModelForCreation(resultOut_mergedScene, DEF_itemOut_mergedScene);

    if (!_radiusFiltered) {_radius = 0;}

    QMap<QString, CT_Scanner*> scansMap = readInFile(_fileName.first(), itemOutModel_scanner, resultOut_individualScenes);

    if (scansMap.size()<=0) {return;}

    float xmin = std::numeric_limits<float>::max();
    float ymin = std::numeric_limits<float>::max();
    float zmin = std::numeric_limits<float>::max();

    float xmax = std::numeric_limits<float>::min();
    float ymax = std::numeric_limits<float>::min();
    float zmax = std::numeric_limits<float>::min();

    QList<CT_PointCloudIndexRegistrationManager::CT_AbstractPCIR> individualScenes;

    int baseProgress = 0;
    int progressIncrement = 100/scansMap.size();

    QMapIterator<QString, CT_Scanner*> it (scansMap);

    while (it.hasNext())
    {
        it.next();


        CT_Reader_ASCRGB reader(it.key());
        CT_Scanner* scanner = it.value();

        if (reader.isValid())
        {
            QPair<CT_Scene*, CT_PointsAttributesColor*> itemsRead = reader.getSceneAndColors(resultOut_individualScenes, itemOutModel_individualScene, itemOutModel_individualSceneColor, _radius);

            CT_Scene* scene = itemsRead.first;
            CT_PointsAttributesColor* colors = itemsRead.second;


            if (scene != NULL)
            {

                CT_StandardItemGroup* groupOut_individualScene = new CT_StandardItemGroup(groupOutModel_individualScenes, resultOut_individualScenes);

                individualScenes.append(scene->getPointCloudIndexRegistered());

                QVector3D min, max;
                scene->getBoundingShape()->getBoundingBox(min, max);
                if (min.x() < xmin) {xmin = min.x();}
                if (max.x() > xmax) {xmax = max.x();}
                if (min.y() < ymin) {ymin = min.y();}
                if (max.y() > ymax) {ymax = max.y();}
                if (min.z() < zmin) {zmin = min.z();}
                if (max.z() > zmax) {zmax = max.z();}

                /* A rétablir quand les problèmes de bounding box seront résolus dans CT_Scene
                if (scene->xMax()>xmax) {xmax = scene->xMax();}
                if (scene->yMin()<ymin) {ymin = scene->yMin();}
                if (scene->yMax()>ymax) {ymax = scene->yMax();}
                if (scene->zMin()<zmin) {zmin = scene->zMin();}
                if (scene->zMax()>zmax) {zmax = scene->zMax();}
                */

                groupOut_individualScene->addItemDrawable(scene);

                if (colors != NULL) {groupOut_individualScene->addItemDrawable(colors);}

                if (scanner != NULL) {groupOut_individualScene->addItemDrawable(scanner);}

                resultOut_individualScenes->addGroup(groupOut_individualScene);
            }


        } else {
            delete scanner;
        }

        baseProgress += progressIncrement;
        setProgress(baseProgress);
    }


    CT_StandardItemGroup* groupOut_mergedScene = new CT_StandardItemGroup(groupOutModel_mergedScene, resultOut_mergedScene);

    CT_Scene *mergedScene = new CT_Scene(itemOutModel_mergedScene, resultOut_mergedScene);
    mergedScene->setBoundingShape(new CT_AxisAlignedBoundingBox(QVector3D(xmin,ymin,zmin), QVector3D(xmax,ymax,zmax)));
    mergedScene->setPointCloudIndexRegistered(PS_REPOSITORY->mergePointCloudContiguous(individualScenes));

    groupOut_mergedScene->addItemDrawable(mergedScene);
    resultOut_mergedScene->addGroup(groupOut_mergedScene);

    setProgress(100);
}

QMap<QString, CT_Scanner*> LVOX_StepLoadInFile::readInFile(QString filename, CT_OutStandardItemDrawableModel* model, Result* result)
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

                    float x = values.at(2).toFloat(&okX);
                    float y = values.at(3).toFloat(&okY);
                    float z = values.at(4).toFloat(&okZ);
                    float res = values.at(5).toFloat(&okRes);
                    float thetaDeb = values.at(6).toFloat(&okThetaDeb);
                    float thetaFin = values.at(7).toFloat(&okThetaFin);
                    float phiDeb = values.at(8).toFloat(&okPhiDeb);
                    float phiFin = values.at(9).toFloat(&okPhiFin);

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
                            CT_Scanner *scanner = new CT_Scanner(model, result, scanID++, QVector3D(x, y, z),QVector3D(0,0,1), hFov, vFov, resolution, resolution, initTheta, initPhi,  clockWise, true);
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
