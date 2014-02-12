/****************************************************************************

 Copyright (C) 2012-2012 Universite de Sherbrooke, Quebec, CANADA
                     All rights reserved.

 Contact :  richard.fournier@usherbrooke.ca
            jean-francois.cote@nrcan-rncan.gc.ca
            joris.ravaglia@gmail.com

 Developers : Joris RAVAGLIA
 Adapted by Alexandre Piboule for Computree 2.0

 This file is part of Computree version 2.0.

 Computree is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Computree is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Computree.  If not, see <http://www.gnu.org/licenses/>.

*****************************************************************************/

#include "lvox_stepcomputelvoxgrids.h"

#include "ct_itemdrawable/model/inModel/ct_instandardgroupmodel.h"
#include "ct_itemdrawable/model/inModel/ct_inzeroormoregroupmodel.h"
#include "ct_itemdrawable/model/inModel/ct_instandarditemdrawablemodel.h"
#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"


// Inclusion of out models
#include "ct_itemdrawable/model/outModel/ct_outstandardgroupmodel.h"
#include "ct_itemdrawable/model/outModel/ct_outstandarditemdrawablemodel.h"
#include "ct_result/model/outModel/ct_outresultmodelgroup.h"

// Inclusion of actions methods
#include "ct_tools/model/ct_outmodelcopyactionaddmodelitemingroup.h"

// Inclusion of standard result class
#include "ct_result/ct_resultgroup.h"

// Inclusion of used ItemDrawable classes
#include "ct_itemdrawable/ct_scene.h"
#include "ct_itemdrawable/ct_scanner.h"
#include "ct_itemdrawable/ct_grid3d.h"
#include "qvector3d.h"

#include "ct_step/abstract/ct_abstractsteploadfile.h"
#include "ct_view/ct_stepconfigurabledialog.h"

#include "tools/lvox_computehitsthread.h"
#include "tools/lvox_computetheoriticalsthread.h"
#include "tools/lvox_computebeforethread.h"
#include "tools/lvox_computedensitythread.h"

#include <QFileInfo>
#include <QDebug>
#include <limits>


#define DEF_SearchInResult "r"
#define DEF_SearchInScene   "sc"
#define DEF_SearchInScan   "sca"
#define DEF_SearchInGroup   "gr"

LVOX_StepComputeLvoxGrids::LVOX_StepComputeLvoxGrids(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    //********************************************//
    //              Attributes of LVox            //
    //********************************************//
    _res = 0.5;
    _effectiveRayThresh = 10;
    _computeDistances = false;
}

QString LVOX_StepComputeLvoxGrids::getStepDescription() const
{
    // Gives the descrption to print in the GUI
    return tr("Discretise l'espace en voxels et calcule un indice de denste par voxel et les distances associees");
}

CT_VirtualAbstractStep* LVOX_StepComputeLvoxGrids::createNewInstance(CT_StepInitializeData &dataInit)
{
    // Creates an instance of this step
    return new LVOX_StepComputeLvoxGrids(dataInit);
}

void LVOX_StepComputeLvoxGrids::createInResultModelListProtected()
{
    CT_InZeroOrMoreGroupModel *rootGroup = new CT_InZeroOrMoreGroupModel();

    // results with Scenes
    CT_InStandardGroupModel *group = new CT_InStandardGroupModel(DEF_SearchInGroup, "Scan");
    rootGroup->addGroup(group);

    CT_InStandardItemDrawableModel *item = new CT_InStandardItemDrawableModel(DEF_SearchInScene, CT_Scene::staticGetType(), tr("Scène"));
    group->addItem(item);

    CT_InStandardItemDrawableModel *item_scanner = new CT_InStandardItemDrawableModel(DEF_SearchInScan, CT_Scanner::staticGetType(), tr("Scanner"));
    group->addItem(item_scanner);


    CT_InResultModelGroupToCopy *resultModel = new CT_InResultModelGroupToCopy(DEF_SearchInResult, rootGroup, tr("Scène(s)"));
    addInResultModel(resultModel);
}

void LVOX_StepComputeLvoxGrids::createOutResultModelListProtected()
{
    // Get IN model corresponding to DEF_resultIn_r
    CT_InResultModelGroupToCopy *resultModel = (CT_InResultModelGroupToCopy*)getInResultModel(DEF_SearchInResult);

    // Create out models for output grids
    CT_OutStandardItemDrawableModel *itemOutModel_bef = new CT_OutStandardItemDrawableModel("", new CT_Grid3D<int>(), tr("Before"));
    CT_OutStandardItemDrawableModel *itemOutModel_theo = new CT_OutStandardItemDrawableModel("", new CT_Grid3D<int>(), tr("Theorical"));
    CT_OutStandardItemDrawableModel *itemOutModel_hits = new CT_OutStandardItemDrawableModel("", new CT_Grid3D<int>(), tr("Hits"));
    CT_OutStandardItemDrawableModel *itemOutModel_density = new CT_OutStandardItemDrawableModel("", new CT_Grid3D<double>(), tr("Density"));

    CT_OutStandardItemDrawableModel *itemOutModel_deltaout = NULL;
    CT_OutStandardItemDrawableModel *itemOutModel_deltain = NULL;
    CT_OutStandardItemDrawableModel *itemOutModel_deltabef = NULL;
    CT_OutStandardItemDrawableModel *itemOutModel_deltatheo = NULL;

    if (_computeDistances)
    {
        itemOutModel_deltaout = new CT_OutStandardItemDrawableModel("", new CT_Grid3D<double>(), tr("DeltaOut"));
        itemOutModel_deltain = new CT_OutStandardItemDrawableModel("", new CT_Grid3D<double>(), tr("DeltaIn"));
        itemOutModel_deltabef = new CT_OutStandardItemDrawableModel("", new CT_Grid3D<int>(), tr("DeltaBefore"));
        itemOutModel_deltatheo = new CT_OutStandardItemDrawableModel("", new CT_Grid3D<int>(), tr("DeltaTheorical"));

    }

    // Create an action list to modify the in model (empty in no modifications)
    QList<CT_AbstractOutModelCopyAction*> actions;

    // Create the action to add the item corresponding to _itemOut_gr_ModelName
    actions << new CT_OutModelCopyActionAddModelItemInGroup(DEF_SearchInGroup, itemOutModel_hits, _hits_ModelName);
    actions << new CT_OutModelCopyActionAddModelItemInGroup(DEF_SearchInGroup, itemOutModel_theo, _theo_ModelName);
    actions << new CT_OutModelCopyActionAddModelItemInGroup(DEF_SearchInGroup, itemOutModel_bef, _bef_ModelName);
    actions << new CT_OutModelCopyActionAddModelItemInGroup(DEF_SearchInGroup, itemOutModel_density, _density_ModelName);

    if (_computeDistances)
    {
        actions << new CT_OutModelCopyActionAddModelItemInGroup(DEF_SearchInGroup, itemOutModel_deltain, _deltain_ModelName);
        actions << new CT_OutModelCopyActionAddModelItemInGroup(DEF_SearchInGroup, itemOutModel_deltaout, _deltaout_ModelName);
        actions << new CT_OutModelCopyActionAddModelItemInGroup(DEF_SearchInGroup, itemOutModel_deltatheo, _deltatheo_ModelName);
        actions << new CT_OutModelCopyActionAddModelItemInGroup(DEF_SearchInGroup, itemOutModel_deltabef, _deltabef_ModelName);
    }

    // Add IN model copy (eventually modified) to OUT results
    addOutResultModelCopy(resultModel, actions);
}

void LVOX_StepComputeLvoxGrids::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    //********************************************//
    //              Attributes of LVox            //
    //********************************************//
    configDialog->addDouble(tr("Size of a voxel"),tr("meters"),0.0001,10000,2, _res );
    configDialog->addInt(tr("Minimum number of effective ray in a voxel to take it into account"),tr(""),-100000,100000, _effectiveRayThresh );
    configDialog->addBool(tr("Compute Distances"), tr(""), tr(""), _computeDistances);

}

void LVOX_StepComputeLvoxGrids::compute()
{   
    CT_InAbstractGroupModel* groupInModel = (CT_InAbstractGroupModel*)getInModelForResearchIfUseCopy(DEF_SearchInResult, DEF_SearchInGroup);
    CT_InAbstractItemDrawableModel* inSceneModel = (CT_InAbstractItemDrawableModel*)getInModelForResearchIfUseCopy(DEF_SearchInResult, DEF_SearchInScene);
    CT_InAbstractItemDrawableModel* inScannerModel = (CT_InAbstractItemDrawableModel*)getInModelForResearchIfUseCopy(DEF_SearchInResult, DEF_SearchInScan);

    // Gets the out result
    CT_ResultGroup* outResult = getOutResultList().first();

    // Get the output items models
    CT_OutStandardItemDrawableModel* itemOutModel_hits = (CT_OutStandardItemDrawableModel*)getOutModelForCreation(outResult, _hits_ModelName.completeName());
    CT_OutStandardItemDrawableModel* itemOutModel_theo = (CT_OutStandardItemDrawableModel*)getOutModelForCreation(outResult, _theo_ModelName.completeName());
    CT_OutStandardItemDrawableModel* itemOutModel_bef = (CT_OutStandardItemDrawableModel*)getOutModelForCreation(outResult, _bef_ModelName.completeName());
    CT_OutStandardItemDrawableModel* itemOutModel_density = (CT_OutStandardItemDrawableModel*)getOutModelForCreation(outResult, _density_ModelName.completeName());

    CT_OutStandardItemDrawableModel* itemOutModel_deltain = NULL;
    CT_OutStandardItemDrawableModel* itemOutModel_deltaout = NULL;
    CT_OutStandardItemDrawableModel* itemOutModel_deltatheo = NULL;
    CT_OutStandardItemDrawableModel* itemOutModel_deltabef = NULL;

    if (_computeDistances)
    {
        itemOutModel_deltain = (CT_OutStandardItemDrawableModel*)getOutModelForCreation(outResult, _deltain_ModelName.completeName());
        itemOutModel_deltaout = (CT_OutStandardItemDrawableModel*)getOutModelForCreation(outResult, _deltaout_ModelName.completeName());
        itemOutModel_deltatheo = (CT_OutStandardItemDrawableModel*)getOutModelForCreation(outResult, _deltatheo_ModelName.completeName());
        itemOutModel_deltabef = (CT_OutStandardItemDrawableModel*)getOutModelForCreation(outResult, _deltabef_ModelName.completeName());
    }

    QList<CT_MonitoredQThread*> baseThreads;
    QList<CT_MonitoredQThread*> densityThreads;

    QMap<CT_AbstractItemGroup*, QPair<const CT_Scene*, const CT_Scanner*> > pointsOfView;

    // Global limits of generated grids
    float xMin = std::numeric_limits<float>::max();
    float yMin = std::numeric_limits<float>::max();
    float zMin = std::numeric_limits<float>::max();
    float xMax = -std::numeric_limits<float>::max();
    float yMax = -std::numeric_limits<float>::max();
    float zMax = -std::numeric_limits<float>::max();

    // on va rechercher tous les groupes contenant des nuages de points (qui ont été choisi par l'utilisateur)
    if(outResult->recursiveBeginIterateGroups(*groupInModel))
    {
        CT_AbstractItemGroup *group;

        // pour chaque groupe contenant un nuage de point
        while(((group = outResult->recursiveNextGroup()) != NULL) && (!isStopped()))
        {
            const CT_Scene* scene = (CT_Scene*)group->findFirstItem(inSceneModel);
            const CT_Scanner* scanner = (CT_Scanner*)group->findFirstItem(inScannerModel);


            if (scene!=NULL && scanner!=NULL)
            {
                pointsOfView.insert(group, QPair<const CT_Scene*, const CT_Scanner*>(scene, scanner));

                QVector3D min, max;
                scene->getBoundingBox(min, max);

                if (min.x() < xMin) {xMin = min.x();}
                if (min.y() < yMin) {yMin = min.y();}
                if (min.z() < zMin) {zMin = min.z();}
                if (max.x() > xMax) {xMax = max.x();}
                if (max.y() > yMax) {yMax = max.y();}
                if (max.z() > zMax) {zMax = max.z();}

                if (scanner->getCenterX() < xMin) {xMin = scanner->getCenterX();}
                if (scanner->getCenterY() < yMin) {yMin = scanner->getCenterY();}
                if (scanner->getCenterZ() < zMin) {zMin = scanner->getCenterZ();}

                if (scanner->getCenterX() > xMax) {xMax = scanner->getCenterX();}
                if (scanner->getCenterY() > yMax) {yMax = scanner->getCenterY();}
                if (scanner->getCenterZ() > zMax) {zMax = scanner->getCenterZ();}
            }
        }
    }

    QMapIterator<CT_AbstractItemGroup*, QPair<const CT_Scene*, const CT_Scanner*> > it(pointsOfView);
    while (it.hasNext() && !isStopped())
    {
        it.next();
        CT_AbstractItemGroup* group = it.key();
        const CT_Scene* scene = it.value().first;
        const CT_Scanner* scanner =it.value().second;

        // Declaring the output grids
        CT_Grid3D<int>*      hitGrid = new CT_Grid3D<int>(itemOutModel_hits, outResult, xMin, yMin, zMin, xMax, yMax, zMax, _res, -1, 0, true);
        CT_Grid3D<int>*      theoriticalGrid = new CT_Grid3D<int>(itemOutModel_theo, outResult, hitGrid->xMin(), hitGrid->yMin(), hitGrid->zMin(), hitGrid->xdim(), hitGrid->ydim(), hitGrid->zdim(), _res, -1, 0);
        CT_Grid3D<int>*      beforeGrid = new CT_Grid3D<int>(itemOutModel_bef, outResult, hitGrid->xMin(), hitGrid->yMin(), hitGrid->zMin(), hitGrid->xdim(), hitGrid->ydim(), hitGrid->zdim(), _res, -1, 0);
        CT_Grid3D<double>*   density = new CT_Grid3D<double>(itemOutModel_density, outResult, hitGrid->xMin(), hitGrid->yMin(), hitGrid->zMin(), hitGrid->xdim(), hitGrid->ydim(), hitGrid->zdim(), _res, -1, 0);

        CT_Grid3D<double>*   deltaInGrid = NULL;
        CT_Grid3D<double>*   deltaOutGrid = NULL;
        CT_Grid3D<double>*   deltaTheoritical = NULL;
        CT_Grid3D<double>*   deltaBefore = NULL;

        group->addItemDrawable(hitGrid);
        group->addItemDrawable(theoriticalGrid);
        group->addItemDrawable(beforeGrid);
        group->addItemDrawable(density);

        if (_computeDistances)
        {
            deltaInGrid = new CT_Grid3D<double>(itemOutModel_deltain, outResult, hitGrid->xMin(), hitGrid->yMin(), hitGrid->zMin(), hitGrid->xdim(), hitGrid->ydim(), hitGrid->zdim(), _res, -1, 0);
            deltaOutGrid = new CT_Grid3D<double>(itemOutModel_deltaout, outResult, hitGrid->xMin(), hitGrid->yMin(), hitGrid->zMin(), hitGrid->xdim(), hitGrid->ydim(), hitGrid->zdim(), _res, -1, 0);
            deltaTheoritical = new CT_Grid3D<double>(itemOutModel_deltatheo, outResult, hitGrid->xMin(), hitGrid->yMin(), hitGrid->zMin(), hitGrid->xdim(), hitGrid->ydim(), hitGrid->zdim(), _res, -1, 0);
            deltaBefore = new CT_Grid3D<double>(itemOutModel_deltabef, outResult, hitGrid->xMin(), hitGrid->yMin(), hitGrid->zMin(), hitGrid->xdim(), hitGrid->ydim(), hitGrid->zdim(), _res, -1, 0);

            group->addItemDrawable(deltaInGrid);
            group->addItemDrawable(deltaOutGrid);
            group->addItemDrawable(deltaTheoritical);
            group->addItemDrawable(deltaBefore);
        }

        LVOX_ComputeHitsThread* hitsThread = new LVOX_ComputeHitsThread(scanner, hitGrid, deltaInGrid, deltaOutGrid, scene, _computeDistances);
        connect(hitsThread, SIGNAL(progressChanged()), this, SLOT(updateProgress()));
        _threadList.append(hitsThread);
        baseThreads.append(hitsThread);

        LVOX_ComputeTheoriticalsThread* theoricalThread = new LVOX_ComputeTheoriticalsThread(scanner, theoriticalGrid, deltaTheoritical, _computeDistances);
        connect(theoricalThread, SIGNAL(progressChanged()), this, SLOT(updateProgress()));
        _threadList.append(theoricalThread);
        baseThreads.append(theoricalThread);

        LVOX_ComputeBeforeThread* beforeThread = new LVOX_ComputeBeforeThread(scanner, beforeGrid, deltaBefore, scene, _computeDistances);
        connect(beforeThread, SIGNAL(progressChanged()), this, SLOT(updateProgress()));
        _threadList.append(beforeThread);
        baseThreads.append(beforeThread);

        LVOX_ComputeDensityThread* densityThread = new LVOX_ComputeDensityThread(density, hitGrid, theoriticalGrid, beforeGrid, _effectiveRayThresh);
        connect(densityThread, SIGNAL(progressChanged()), this, SLOT(updateProgress()));
        _threadList.append(densityThread);
        densityThreads.append(densityThread);
    }

    int size = baseThreads.size();

    for (int i = 0 ; i < size ; ++i)
    {
        baseThreads.at(i)->start();
    }

    for (int i = 0 ; i < size ; ++i)
    {
        baseThreads.at(i)->wait();
        disconnect(baseThreads.at(i), SIGNAL(progressChanged()), this, SLOT(updateProgress()));
        updateProgress();
    }

    size = densityThreads.size();

    for (int i = 0 ; i < size ; ++i)
    {
        densityThreads.at(i)->start();

    }

    for (int i = 0 ; i < size ; ++i)
    {
        densityThreads.at(i)->wait();
        disconnect(densityThreads.at(i), SIGNAL(progressChanged()), this, SLOT(updateProgress()));
        updateProgress();
    }

    qDeleteAll(_threadList);

    setProgress(100);
}

void LVOX_StepComputeLvoxGrids::updateProgress()
{
    int progress = 0;

    int size = _threadList.size();
    for (int i = 0 ; i < size ; ++i)
    {
        progress += _threadList.at(i)->getProgress();
    }

    progress /= size;
    setProgress(progress);
}

