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


#define DEF_SearchInResult "r"
#define DEF_SearchInScene   "sc"
#define DEF_SearchInGroup   "gr"

LVOX_StepComputeLvoxGrids::LVOX_StepComputeLvoxGrids(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    //********************************************//
    //              Attributes of LVox            //
    //********************************************//
    _res = 0.5;
    _effectiveRayThresh = 10;
    _computeDistances = true;

    //********************************************//
    //           Attributes of the scanner        //
    //********************************************//
    _scanPosX = 0;
    _scanPosY = 0;
    _scanPosZ = 0;
    _scanHFov = 360;
    _scanVFov = 150;
    _scanInitTheta = 0;
    _scanInitPhi = 0;
    _scanHRes = 0.036;
    _scanVRes = 0.036;
    _scanClockWise = false;
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

    CT_InResultModelGroupToCopy *resultModel = new CT_InResultModelGroupToCopy(DEF_SearchInResult, rootGroup, tr("Scène(s)"));
    addInResultModel(resultModel);
}

void LVOX_StepComputeLvoxGrids::createOutResultModelListProtected()
{
    // Get IN model corresponding to DEF_resultIn_r
    CT_InResultModelGroupToCopy *resultModel = (CT_InResultModelGroupToCopy*)getInResultModel(DEF_SearchInResult);

    // Create out models for output grids
    CT_OutStandardItemDrawableModel *itemOutModel_scan = new CT_OutStandardItemDrawableModel("", new CT_Scanner(), tr("Scanner"));
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
    actions << new CT_OutModelCopyActionAddModelItemInGroup(DEF_SearchInGroup, itemOutModel_scan, _scan_ModelName);
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
    configDialog->addDouble(tr("Minimum number of effective ray in a voxel to take it into account"),tr(""),-100000,100000,2, _effectiveRayThresh );
    configDialog->addBool(tr("Compute Distances"), tr(""), tr(""), _computeDistances);

    //********************************************//
    //           Attributes of the scanner        //
    //********************************************//
    configDialog->addDouble(tr("Position du scanner x"),tr("metres"),-10000,10000,2, _scanPosX );
    configDialog->addDouble(tr("Position du scanner y"),tr("metres"),-10000,10000,2, _scanPosY );
    configDialog->addDouble(tr("Position du scanner z"),tr("metres"),-10000,10000,2, _scanPosZ );

    configDialog->addDouble(tr("Champ de vue horizontal du scanner"),tr("degres"),0.0001,360,2, _scanHFov );
    configDialog->addDouble(tr("Champ de vue vertical du scanner"),tr("degres"),0.0001,360,2, _scanVFov );

    configDialog->addDouble(tr("Theta initial du scanner"),tr("degres"),-359.99,359.99,2, _scanInitTheta );
    configDialog->addDouble(tr("Phi initial du scanner"),tr("degres"),0,180,2, _scanInitPhi );

    configDialog->addDouble(tr("Resolution angulaire horizontale du scanner"),tr("degres"),0.0001,10000,3, _scanHRes );
    configDialog->addDouble(tr("Resolution angulaire verticale du scanner"),tr("degres"),0.0001,10000,3, _scanVRes );

    configDialog->addBool("Scanner sens horaire","","",_scanClockWise);
}

void LVOX_StepComputeLvoxGrids::compute()
{   
    // Get the group model corresponding to DEF_SearchInGroup
    CT_InAbstractGroupModel* groupInModel = (CT_InAbstractGroupModel*)getInModelForResearchIfUseCopy(DEF_SearchInResult, DEF_SearchInGroup);
    // Get the group model corresponding to DEF_SearchInScene
    CT_InAbstractItemDrawableModel* inSceneModel = (CT_InAbstractItemDrawableModel*)getInModelForResearchIfUseCopy(DEF_SearchInResult, DEF_SearchInScene);

    // Gets the out result
    CT_ResultGroup* outResult = getOutResultList().first();

    // Get the output items models
    CT_OutStandardItemDrawableModel* itemOutModel_scan = (CT_OutStandardItemDrawableModel*)getOutModelForCreation(outResult, _scan_ModelName.completeName());
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

    // on va rechercher tous les groupes contenant des nuages de points (qui ont été choisi par l'utilisateur)
    if(outResult->recursiveBeginIterateGroups(*groupInModel))
    {
        CT_AbstractItemGroup *group;

        // pour chaque groupe contenant un nuage de point
        while(((group = outResult->recursiveNextGroup()) != NULL) && (!isStopped()))
        {
            const CT_Scene* scene = (CT_Scene*)group->findFirstItem(inSceneModel);
            if (scene!=NULL)
            {
                // Creating a new scanner from the input parameters
                CT_Scanner* scanner = new CT_Scanner(itemOutModel_scan,
                                                     outResult,
                                                     0,
                                                     QVector3D(_scanPosX, _scanPosY, _scanPosZ),
                                                     QVector3D(0,0,1),
                                                     _scanHFov,
                                                     _scanVFov,
                                                     _scanHRes,
                                                     _scanVRes,
                                                     _scanInitTheta,
                                                     _scanInitPhi,
                                                     _scanClockWise);

                // Declaring the output grids
                CT_Grid3D<int>*      hitGrid = new CT_Grid3D<int>(itemOutModel_hits, outResult, scene->xMin(), scene->yMin(), scene->zMin(), scene->xMax(), scene->yMax(), scene->zMax(), _res, -1, 0, true);
                CT_Grid3D<int>*      theoriticalGrid = new CT_Grid3D<int>(itemOutModel_theo, outResult, hitGrid->xMin(), hitGrid->yMin(), hitGrid->zMin(), hitGrid->xdim(), hitGrid->ydim(), hitGrid->zdim(), _res, -1, 0);
                CT_Grid3D<int>*      beforeGrid = new CT_Grid3D<int>(itemOutModel_bef, outResult, hitGrid->xMin(), hitGrid->yMin(), hitGrid->zMin(), hitGrid->xdim(), hitGrid->ydim(), hitGrid->zdim(), _res, -1, 0);
                CT_Grid3D<double>*   density = new CT_Grid3D<double>(itemOutModel_density, outResult, hitGrid->xMin(), hitGrid->yMin(), hitGrid->zMin(), hitGrid->xdim(), hitGrid->ydim(), hitGrid->zdim(), _res, -1, 0);

                CT_Grid3D<double>*   deltaInGrid = NULL;
                CT_Grid3D<double>*   deltaOutGrid = NULL;
                CT_Grid3D<double>*   deltaTheoritical = NULL;
                CT_Grid3D<double>*   deltaBefore = NULL;

                group->addItemDrawable(scanner);
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
                hitsThread->start();
                _threadList.append(hitsThread);
                baseThreads.append(hitsThread);

                LVOX_ComputeTheoriticalsThread* theoricalThread = new LVOX_ComputeTheoriticalsThread(scanner, theoriticalGrid, deltaTheoritical, _computeDistances);
                connect(theoricalThread, SIGNAL(progressChanged()), this, SLOT(updateProgress()));
                theoricalThread->start();
                _threadList.append(theoricalThread);
                baseThreads.append(theoricalThread);

                LVOX_ComputeBeforeThread* beforeThread = new LVOX_ComputeBeforeThread(scanner, beforeGrid, deltaBefore, scene, _computeDistances);
                connect(beforeThread, SIGNAL(progressChanged()), this, SLOT(updateProgress()));
                beforeThread->start();
                _threadList.append(beforeThread);
                baseThreads.append(beforeThread);

                LVOX_ComputeDensityThread* densityThread = new LVOX_ComputeDensityThread(density, hitGrid, theoriticalGrid, beforeGrid, _effectiveRayThresh);
                connect(densityThread, SIGNAL(progressChanged()), this, SLOT(updateProgress()));
                _threadList.append(densityThread);
                densityThreads.append(densityThread);
            }
        }

        int size = baseThreads.size();
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
    }

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

