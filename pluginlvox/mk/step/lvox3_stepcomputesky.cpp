#include "lvox3_stepcomputesky.h"

#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"
#include "ct_result/ct_resultgroup.h"
#include "ct_attributes/ct_stditemattributet.h"
#include "ct_itemdrawable/ct_scene.h"
#include "ct_view/ct_stepconfigurabledialog.h"
#include "mk/tools/lvox3_gridtype.h"
#include "mk/tools/lvox3_gridtools.h"
#include "mk/tools/lvox3_computehits.h"

#define DEF_SearchInResult      "r"
#define DEF_SearchInScene       "sc"
#define DEF_SearchInGroup       "gr"

LVOX3_StepComputeSky::LVOX3_StepComputeSky(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    m_minimumNumberOfPoints = 5;
    m_gridResolution = 0.5;
}

QString LVOX3_StepComputeSky::getStepDescription() const
{
    return tr("Calcul une grille ciel (LVOX 3)");
}

CT_VirtualAbstractStep* LVOX3_StepComputeSky::createNewInstance(CT_StepInitializeData &dataInit)
{
    // Creates an instance of this step
    return new LVOX3_StepComputeSky(dataInit);
}

void LVOX3_StepComputeSky::createInResultModelListProtected()
{
    // We must have
    // - a scene

    CT_InResultModelGroupToCopy *resultScan = createNewInResultModelForCopy(DEF_SearchInResult, tr("Scène"), "", true);
    resultScan->setZeroOrMoreRootGroup();
    resultScan->addGroupModel("", DEF_SearchInGroup, CT_AbstractItemGroup::staticGetType(), tr("Scan"));
    resultScan->addItemModel(DEF_SearchInGroup, DEF_SearchInScene, CT_Scene::staticGetType(), tr("Scène"));
}

void LVOX3_StepComputeSky::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addInt(tr("Nombre minimum de points"), "", 0, 100000, m_minimumNumberOfPoints);
    configDialog->addDouble(tr("Resolution de la grille"), tr("mètre"), 0.0001, 10000, 2, m_gridResolution);
}

void LVOX3_StepComputeSky::createOutResultModelListProtected()
{
    // create a new OUT result that is a copy of the IN result selected by the user
    CT_OutResultModelGroupToCopyPossibilities *resultModel = createNewOutResultModelToCopy(DEF_SearchInResult);

    if(resultModel != NULL) {
        resultModel->addItemModel(DEF_SearchInGroup, m_outDSMModelName, new lvox::SkyRaster(), tr("MNC"));
        resultModel->addItemModel(DEF_SearchInGroup, m_outHitGridModelName, new lvox::Grid3Di(), tr("Sky hit grid"));
        resultModel->addItemAttributeModel(m_outDSMModelName, m_outZMaxModelName, new CT_StdItemAttributeT<double>(CT_AbstractCategory::staticInitDataZ()), tr("Z level"), tr("Z level of the sky"));
    }
}

void LVOX3_StepComputeSky::compute()
{
    CT_ResultGroup* outResult = getOutResultList().first();

    CT_ResultGroupIterator itGrp(outResult, this, DEF_SearchInGroup);

    while(itGrp.hasNext() && !isStopped()) {
        CT_StandardItemGroup *group = dynamic_cast<CT_StandardItemGroup*>((CT_AbstractItemGroup*)itGrp.next());

        CT_Scene* scene = dynamic_cast<CT_Scene*>(group->firstItemByINModelName(this, DEF_SearchInScene));

        if(scene) {
            Eigen::Vector3d minBBox, maxBBox;
            scene->getBoundingBox(minBBox, maxBBox);

            minBBox.array() -= m_gridResolution;
            maxBBox.array() += m_gridResolution;

            lvox::Grid3Di* hitGrid = lvox::Grid3Di::createGrid3DFromXYZCoords(m_outHitGridModelName.completeName(), outResult, minBBox.x(), minBBox.y(), minBBox.z(), maxBBox.x(), maxBBox.y(), maxBBox.z(), m_gridResolution, 0, 0, true);

            LVOX3_ComputeHits computeHits(NULL, scene->getPointCloudIndex(), hitGrid);

            connect(this, SIGNAL(stopped()), &computeHits, SLOT(cancel()), Qt::DirectConnection);
            connect(&computeHits, SIGNAL(progressChanged(int)), this, SLOT(workerProgressChanged(int)), Qt::DirectConnection);

            computeHits.compute();

            double maxSize = qMax(maxBBox.x()-minBBox.x(), maxBBox.y()-minBBox.y())/2.0;
            lvox::SkyRaster *skyRaster = lvox::SkyRaster::createImage2DFromXYCoords(m_outDSMModelName.completeName(), outResult, minBBox.x(), minBBox.y(), minBBox.x()+maxSize, minBBox.y()+maxSize, maxSize, hitGrid->maxZ(), 0, hitGrid->minZ());

            size_t index = 0;
            size_t nCells = hitGrid->nCells();
            size_t level, indice;
            double maxSkyLevel = hitGrid->minZ();

            LVOX3_GridTools gridTools(hitGrid);


            for(size_t col = 0; col<hitGrid->xdim()  && !isStopped(); ++col) {
                for(size_t lin = 0; lin<hitGrid->ydim() && !isStopped(); ++lin) {

                    level = hitGrid->zdim();
                    do {
                        --level;
                        gridTools.computeGridIndexForColLinLevel(col, lin, level, indice);
                        if(hitGrid->valueAtIndex(indice) > m_minimumNumberOfPoints) {
                            const double value = hitGrid->minZ()+((level+1)*m_gridResolution);
                            maxSkyLevel = qMax(value, maxSkyLevel);
                            break;
                        }

                        setProgress(50 + ((index*50)/nCells));
                        ++index;
                    }while((level > 0) && !isStopped());
                }
            }

            for(size_t i=0; i<skyRaster->nCells(); ++i)
                skyRaster->setValueAtIndex(i, maxSkyLevel);

            skyRaster->setlevel(maxSkyLevel);
            skyRaster->computeMinMax();

            group->addItemDrawable(skyRaster);

            skyRaster->addItemAttribute(new CT_StdItemAttributeT<double>(m_outZMaxModelName.completeName(),
                                                                    CT_AbstractCategory::staticInitDataZ(),
                                                                    outResult,
                                                                    maxSkyLevel));
            group->addItemDrawable(hitGrid);
        }
    }
}

void LVOX3_StepComputeSky::workerProgressChanged(int p)
{
    setProgress(p/2);
}

