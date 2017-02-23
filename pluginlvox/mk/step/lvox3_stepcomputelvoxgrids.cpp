#include "lvox3_stepcomputelvoxgrids.h"

#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"

#include "ct_result/ct_resultgroup.h"

#include "ct_itemdrawable/ct_scene.h"
#include "ct_itemdrawable/ct_scanner.h"
#include "ct_itemdrawable/ct_shootingpatternd.h"
#include "ct_itemdrawable/ct_grid3d.h"
#include "ct_itemdrawable/abstract/ct_abstractimage2d.h"

#include "ct_view/ct_stepconfigurabledialog.h"
#include "ct_view/ct_buttongroup.h"

#include "mk/tools/worker/lvox3_filtervoxelsbyzvaluesofraster.h"
#include "mk/tools/worker/lvox3_computehits.h"
#include "mk/tools/worker/lvox3_computebefore.h"
#include "mk/tools/worker/lvox3_computetheoriticals.h"
#include "mk/tools/worker/lvox3_computeall.h"
#include "mk/tools/lvox3_computelvoxgridspreparator.h"
#include "mk/tools/lvox3_gridtype.h"
#include "mk/tools/lvox3_errorcode.h"

#define DEF_SearchInResult      "r"
#define DEF_SearchInGroup       "gr"
#define DEF_SearchInScene       "sc"
#define DEF_SearchInScan        "sca"
#define DEF_SearchInShotPattern "pat"
#define DEF_SearchInMNT         "mnt"
#define DEF_SearchInSky         "sky"

LVOX3_StepComputeLvoxGrids::LVOX3_StepComputeLvoxGrids(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    m_resolution = 0.5;
    m_computeDistances = false;

    m_gridMode = lvox::BoundingBoxOfTheScene;
    m_coordinates.x() = -20.0;
    m_coordinates.y() = -20.0;
    m_coordinates.z() = -10.0;

    m_dimensions.x() = 80;
    m_dimensions.y() = 80;
    m_dimensions.z() = 80;
}

QString LVOX3_StepComputeLvoxGrids::getStepDescription() const
{
    return tr("2 - Calculer les grilles LVOX (LVOX 3)");
}

CT_VirtualAbstractStep* LVOX3_StepComputeLvoxGrids::createNewInstance(CT_StepInitializeData &dataInit)
{
    // Creates an instance of this step
    return new LVOX3_StepComputeLvoxGrids(dataInit);
}

void LVOX3_StepComputeLvoxGrids::createInResultModelListProtected()
{
    // We must have
    // - a scene
    // - a scanner OR a shooting pattern to simulate shots (at least one is obligatory)
    // - an MNT (optionnal)

    CT_InResultModelGroupToCopy *resultScan = createNewInResultModelForCopy(DEF_SearchInResult, tr("Scène"), "", true);
    resultScan->setZeroOrMoreRootGroup();
    resultScan->addGroupModel("", DEF_SearchInGroup, CT_AbstractItemGroup::staticGetType(), tr("Scan"));
    resultScan->addItemModel(DEF_SearchInGroup, DEF_SearchInScene, CT_Scene::staticGetType(), tr("Scène"));

    // TODO : in ComputreeV5 set the scanner and the pattern in an exclusive group
    resultScan->addItemModel(DEF_SearchInGroup, DEF_SearchInScan, CT_Scanner::staticGetType(), tr("Scanner"), tr("The scanner is used to simulate the shooting pattern. If it was not present choose at least a shooting pattern !"), CT_InAbstractModel::C_ChooseOneIfMultiple, CT_InAbstractModel::F_IsOptional);
    resultScan->addItemModel(DEF_SearchInGroup, DEF_SearchInShotPattern, CT_ShootingPatternD::staticGetType(), tr("Shooting pattern"), tr("The shooting pattern is used to simulate it. If it was not present choose at least a scanner !"), CT_InAbstractModel::C_ChooseOneIfMultiple, CT_InAbstractModel::F_IsOptional);

    resultScan->addItemModel(DEF_SearchInGroup, DEF_SearchInMNT, CT_AbstractImage2D::staticGetType(), tr("MNT"), "", CT_InAbstractModel::C_ChooseOneIfMultiple, CT_InAbstractModel::F_IsOptional);
    resultScan->addItemModel(DEF_SearchInGroup, DEF_SearchInSky, CT_AbstractImage2D::staticGetType(), tr("MNC"), "", CT_InAbstractModel::C_ChooseOneIfMultiple, CT_InAbstractModel::F_IsOptional);
}

void LVOX3_StepComputeLvoxGrids::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    //********************************************//
    //              Attributes of LVox            //
    //********************************************//
    configDialog->addDouble(tr("Resolution of the grids"),tr("meters"),0.0001,10000,2, m_resolution );
    configDialog->addBool("", "", tr("Compute Distances"), m_computeDistances);
    configDialog->addEmpty();

    configDialog->addText(tr("Reference for (minX, minY, minZ) corner of the grid :"),"", "");

    CT_ButtonGroup &bg_gridMode = configDialog->addButtonGroup(m_gridMode);
    configDialog->addExcludeValue("", "", tr("Default mode : Bounding box of the scene"), bg_gridMode, 0);
    configDialog->addExcludeValue("", "", tr("Custom mode : Relative to folowing coordinates:"), bg_gridMode, 1);
    configDialog->addExcludeValue("", "", tr("Custom mode : Relative to folowing coordinates + custom dimensions:"), bg_gridMode, 2);
    configDialog->addExcludeValue("", "", tr("Custom mode : centered on folowing coordinates + custom dimensions:"), bg_gridMode, 3);

#ifdef CT_LARCHIHEADER_H
    configDialog->addExcludeValue("", "", tr("Automatic mode : use grid paramters from grid file"), bg_gridMode, 4);
    configDialog->addFileChoice(tr("Choisir le fichier .grid"), CT_FileChoiceButton::OneExistingFile, "Fichier L-Architect (*.grid)", m_gridFilePath);
#endif

    float maxFloat = std::numeric_limits<float>::max();
    float minFloat = -maxFloat;

    configDialog->addDouble(tr("X coordinate:"), "", minFloat, maxFloat, 4, m_coordinates.x());
    configDialog->addDouble(tr("Y coordinate:"), "", minFloat, maxFloat, 4, m_coordinates.y());
    configDialog->addDouble(tr("Z coordinate:"), "", minFloat, maxFloat, 4, m_coordinates.z());

    configDialog->addInt(tr("X dimension:"), "", 1, 1000, m_dimensions.x());
    configDialog->addInt(tr("Y dimension:"), "", 1, 1000, m_dimensions.y());
    configDialog->addInt(tr("Z dimension:"), "", 1, 1000, m_dimensions.z());
}

void LVOX3_StepComputeLvoxGrids::createOutResultModelListProtected()
{
    // create a new OUT result that is a copy of the IN result selected by the user
    CT_OutResultModelGroupToCopyPossibilities *res = createNewOutResultModelToCopy(DEF_SearchInResult);

    if (res != NULL)
    {
        // Add it all grids : Ni, Nb, Nt, Density
        res->addItemModel(DEF_SearchInGroup, _hits_ModelName, new lvox::Grid3Di(), tr("Hits"));
        res->addItemAttributeModel(_hits_ModelName, _NiFlag_ModelName, new CT_StdItemAttributeT<bool>("LVOX_GRD_NI"), tr("isNi"));

        res->addItemModel(DEF_SearchInGroup, _theo_ModelName, new lvox::Grid3Di(), tr("Theoretical"));
        res->addItemAttributeModel(_theo_ModelName, _NtFlag_ModelName, new CT_StdItemAttributeT<bool>("LVOX_GRD_NT"), tr("isNt"));

        res->addItemModel(DEF_SearchInGroup, _bef_ModelName, new lvox::Grid3Di(), tr("Before"));
        res->addItemAttributeModel(_bef_ModelName, _NbFlag_ModelName, new CT_StdItemAttributeT<bool>("LVOX_GRD_NB"), tr("isNb"));

        // and if must compute distance, grids with distance
        if (m_computeDistances)
        {
            res->addItemModel(DEF_SearchInGroup, _deltain_ModelName, new lvox::Grid3Df(), tr("DeltaIn"));
            res->addItemModel(DEF_SearchInGroup, _deltaout_ModelName, new lvox::Grid3Df(), tr("DeltaOut"));
            res->addItemModel(DEF_SearchInGroup, _deltatheo_ModelName, new lvox::Grid3Df(), tr("Deltatheoretical"));
            res->addItemModel(DEF_SearchInGroup, _deltabef_ModelName, new lvox::Grid3Df(), tr("DeltaBefore"));
        }
    }
}

void LVOX3_StepComputeLvoxGrids::compute()
{
    CT_ResultGroup* outResult = getOutResultList().first();

    CT_ResultGroupIterator itGrp(outResult, this, DEF_SearchInGroup);

    LVOX3_ComputeLVOXGridsPreparator::Coordinates coo;
    coo.coordinate = m_coordinates;
    coo.dimension = m_dimensions.cast<double>();

    LVOX3_ComputeLVOXGridsPreparator p;
    LVOX3_ComputeLVOXGridsPreparator::Result pRes = p.prepare(this,
                                                              itGrp,
                                                              DEF_SearchInScene,
                                                              DEF_SearchInScan,
                                                              DEF_SearchInShotPattern,
                                                              DEF_SearchInMNT,
                                                              DEF_SearchInSky,
                                                              m_resolution,
                                                              (lvox::GridMode)m_gridMode,
                                                              coo,
                                                              m_gridFilePath.isEmpty() ? "" : m_gridFilePath.first());


    if(pRes.valid) {
        LVOX3_ComputeAll workersManager;
        LVOX3_ComputeLVOXGridsPreparator::Result::ToComputeCollectionIterator it(pRes.elementsToCompute);

        while (it.hasNext()
               && !isStopped())
        {
            it.next();
            CT_AbstractItemGroup* group = it.key();
            const LVOX3_ComputeLVOXGridsPreparator::ToCompute& tc = it.value();

            // Declaring the output grids
            lvox::Grid3Di*      hitGrid = lvox::Grid3Di::createGrid3DFromXYZCoords(_hits_ModelName.completeName(), outResult, pRes.minBBox.x(), pRes.minBBox.y(), pRes.minBBox.z(), pRes.maxBBox.x(), pRes.maxBBox.y(), pRes.maxBBox.z(), m_resolution, lvox::Max_Error_Code, 0, true);
            lvox::Grid3Di*      theoriticalGrid = new lvox::Grid3Di(_theo_ModelName.completeName(), outResult, hitGrid->minX(), hitGrid->minY(), hitGrid->minZ(), hitGrid->xdim(), hitGrid->ydim(), hitGrid->zdim(), m_resolution, lvox::Max_Error_Code, 0);
            lvox::Grid3Di*      beforeGrid = new lvox::Grid3Di(_bef_ModelName.completeName(), outResult, hitGrid->minX(), hitGrid->minY(), hitGrid->minZ(), hitGrid->xdim(), hitGrid->ydim(), hitGrid->zdim(), m_resolution, lvox::Max_Error_Code, 0);

            QList<CT_AbstractGrid3D*> allGrids;
            allGrids.append(hitGrid);
            allGrids.append(theoriticalGrid);
            allGrids.append(beforeGrid);

            hitGrid->addItemAttribute(new CT_StdItemAttributeT<bool>(_NiFlag_ModelName.completeName(), "LVOX_GRD_NI", outResult, true));
            theoriticalGrid->addItemAttribute(new CT_StdItemAttributeT<bool>(_NtFlag_ModelName.completeName(), "LVOX_GRD_NT", outResult, true));
            beforeGrid->addItemAttribute(new CT_StdItemAttributeT<bool>(_NbFlag_ModelName.completeName(), "LVOX_GRD_NB", outResult, true));

            lvox::Grid3Df*   deltaInGrid = NULL;
            lvox::Grid3Df*   deltaOutGrid = NULL;
            lvox::Grid3Df*   deltaTheoritical = NULL;
            lvox::Grid3Df*   deltaBefore = NULL;

            group->addItemDrawable(hitGrid);
            group->addItemDrawable(theoriticalGrid);
            group->addItemDrawable(beforeGrid);

            if (m_computeDistances)
            {
                deltaInGrid = new lvox::Grid3Df(_deltain_ModelName.completeName(), outResult, hitGrid->minX(), hitGrid->minY(), hitGrid->minZ(), hitGrid->xdim(), hitGrid->ydim(), hitGrid->zdim(), m_resolution, -1, 0);
                deltaOutGrid = new lvox::Grid3Df(_deltaout_ModelName.completeName(), outResult, hitGrid->minX(), hitGrid->minY(), hitGrid->minZ(), hitGrid->xdim(), hitGrid->ydim(), hitGrid->zdim(), m_resolution, -1, 0);
                deltaTheoritical = new lvox::Grid3Df(_deltatheo_ModelName.completeName(), outResult, hitGrid->minX(), hitGrid->minY(), hitGrid->minZ(), hitGrid->xdim(), hitGrid->ydim(), hitGrid->zdim(), m_resolution, -1, 0);
                deltaBefore = new lvox::Grid3Df(_deltabef_ModelName.completeName(), outResult, hitGrid->minX(), hitGrid->minY(), hitGrid->minZ(), hitGrid->xdim(), hitGrid->ydim(), hitGrid->zdim(), m_resolution, -1, 0);

                // No necessary to filter distance grids (filter voxels below mnt) because
                // if the value of a grid is under 0 (error code) it will be replicated by
                // worker in distance grid.

                /*allGrids.append(deltaInGrid);
                allGrids.append(deltaOutGrid);
                allGrids.append(deltaTheoritical);
                allGrids.append(deltaBefore);*/

                group->addItemDrawable(deltaInGrid);
                group->addItemDrawable(deltaOutGrid);
                group->addItemDrawable(deltaTheoritical);
                group->addItemDrawable(deltaBefore);
            }

            LVOX3_FilterVoxelsByZValuesOfRaster* filterVoxelsBelowMNTWorker = NULL;
            LVOX3_FilterVoxelsByZValuesOfRaster* filterVoxelsInSkyWorker = NULL;

            if(tc.mnt != NULL)
                filterVoxelsBelowMNTWorker = new LVOX3_FilterVoxelsByZValuesOfRaster(allGrids, tc.mnt, LVOX3_FilterVoxelsByZValuesOfRaster::Below, lvox::MNT);

            if(tc.sky != NULL)
                filterVoxelsInSkyWorker = new LVOX3_FilterVoxelsByZValuesOfRaster(allGrids, tc.sky, LVOX3_FilterVoxelsByZValuesOfRaster::Above, lvox::Sky);

            LVOX3_ComputeHits* hitsWorker = new LVOX3_ComputeHits(tc.pattern, tc.scene->getPointCloudIndex(), hitGrid, deltaInGrid, deltaOutGrid);
            LVOX3_ComputeTheoriticals* theoriticalWorker = new LVOX3_ComputeTheoriticals(tc.pattern, theoriticalGrid, deltaTheoritical);
            LVOX3_ComputeBefore* beforeWorker = new LVOX3_ComputeBefore(tc.pattern, tc.scene->getPointCloudIndex(), beforeGrid, deltaBefore);

            if(filterVoxelsBelowMNTWorker != NULL)
                workersManager.addWorker(0, filterVoxelsBelowMNTWorker);

            if(filterVoxelsInSkyWorker != NULL)
                workersManager.addWorker(0, filterVoxelsInSkyWorker);

            workersManager.addWorker(1, hitsWorker);
            workersManager.addWorker(1, theoriticalWorker);
            workersManager.addWorker(1, beforeWorker);
        }

        connect(&workersManager, SIGNAL(progressChanged(int)), this, SLOT(progressChanged(int)), Qt::DirectConnection);
        connect(this, SIGNAL(stopped()), &workersManager, SLOT(cancel()), Qt::DirectConnection);

        workersManager.compute();
    }
}

void LVOX3_StepComputeLvoxGrids::progressChanged(int p)
{
    setProgress(p);
}

