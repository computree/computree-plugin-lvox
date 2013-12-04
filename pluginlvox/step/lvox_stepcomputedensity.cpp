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

#include "lvox_stepcomputedensity.h"

#include "ct_itemdrawable/model/inModel/ct_inoneormoregroupmodel.h"
#include "ct_itemdrawable/model/inModel/ct_instandarditemdrawablemodel.h"
#include "ct_result/model/inModel/ct_inresultmodelgroup.h"

// Inclusion of out models
#include "ct_itemdrawable/model/outModel/ct_outstandardgroupmodel.h"
#include "ct_itemdrawable/model/outModel/ct_outstandardgroupmodel.h"
#include "ct_itemdrawable/model/outModel/ct_outstandarditemdrawablemodel.h"
#include "ct_result/model/outModel/ct_outresultmodelgroup.h"

// Inclusion of standard result class
#include "ct_result/ct_resultgroup.h"

// Inclusion of used ItemDrawable classes
#include "ct_itemdrawable/ct_scene.h"
#include "ct_itemdrawable/ct_scanner.h"
#include "ct_itemdrawable/ct_grid3d.h"
#include "qvector3d.h"

#include "ct_step/abstract/ct_abstractsteploadfile.h"
#include "ct_view/ct_stepconfigurabledialog.h"

#include "algorithmewoo.h"
#include "tools/lvox_computehitsthread.h"
#include "tools/lvox_computetheoriticalsthread.h"
#include "tools/lvox_computebeforethread.h"

#include <QFileInfo>
#include <QDebug>


#define DEF_SearchInResult "r"
#define DEF_SearchInScene   "sc"

// Alias for indexing out models
#define DEF_resultOut_grid "grid"
#define DEF_groupOut_base "base"
#define DEF_itemOut_deltaout "deltaout"
#define DEF_itemOut_deltain "deltain"
#define DEF_itemOut_deltabef "deltabef"
#define DEF_itemOut_deltatheo "deltatheo"
#define DEF_itemOut_bef "bef"
#define DEF_itemOut_theo "theo"
#define DEF_itemOut_hits "hits"
#define DEF_itemOut_density "density"
#define DEF_itemOut_scan "scan"

LVOX_StepComputeDensity::LVOX_StepComputeDensity(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    //********************************************//
    //              Attributes of LVox            //
    //********************************************//
    _res = 0.5;
    _intensityThresh = 10;
    _greaterThanThresh = true;

    _effectiveRayThresh = 10;

    //********************************************//
    //           Attributes of the scanner        //
    //********************************************//
    _scanPosX = 0;
    _scanPosY = 0;
    _scanPosZ = 0;
    _scanHFov = 360;
    _scanVFov = 150;
    _scanInitTheta = 0;
    _scanInitPhi = -60;
    _scanHRes = 0.036;
    _scanVRes = 0.036;
    _scanClockWise = false;
}

QString LVOX_StepComputeDensity::getStepDescription() const
{
    // Gives the descrption to print in the GUI
    return tr("Discretise l'espace en voxels et calcule un indice de denste par voxel et les distances associees");
}

CT_VirtualAbstractStep* LVOX_StepComputeDensity::createNewInstance(CT_StepInitializeData &dataInit)
{
    // Creates an instance of this step
    return new LVOX_StepComputeDensity(dataInit);
}

void LVOX_StepComputeDensity::createInResultModelListProtected()
{
    // results with Scenes
    CT_InOneOrMoreGroupModel *group = new CT_InOneOrMoreGroupModel();

    CT_InStandardItemDrawableModel *item = new CT_InStandardItemDrawableModel(DEF_SearchInScene, CT_Scene::staticGetType(), tr("Scène"));
    group->addItem(item);

    CT_InResultModelGroup *resultModel = new CT_InResultModelGroup(DEF_SearchInResult, group, tr("Scène(s)"));
    addInResultModel(resultModel);
}

void LVOX_StepComputeDensity::createOutResultModelListProtected()
{
    CT_OutStandardGroupModel *groupOutModel_base = new CT_OutStandardGroupModel(DEF_groupOut_base, new CT_StandardItemGroup(), tr("base"));

    CT_OutStandardItemDrawableModel *itemOutModel_deltaout = new CT_OutStandardItemDrawableModel(DEF_itemOut_deltaout, new CT_Grid3D<double>(), tr("DeltaOut"));
    CT_OutStandardItemDrawableModel *itemOutModel_deltain = new CT_OutStandardItemDrawableModel(DEF_itemOut_deltain, new CT_Grid3D<double>(), tr("DeltaIn"));
    CT_OutStandardItemDrawableModel *itemOutModel_deltabef = new CT_OutStandardItemDrawableModel(DEF_itemOut_deltabef, new CT_Grid3D<int>(), tr("DeltaBefore"));
    CT_OutStandardItemDrawableModel *itemOutModel_deltatheo = new CT_OutStandardItemDrawableModel(DEF_itemOut_deltatheo, new CT_Grid3D<int>(), tr("DeltaTheorical"));
    CT_OutStandardItemDrawableModel *itemOutModel_bef = new CT_OutStandardItemDrawableModel(DEF_itemOut_bef, new CT_Grid3D<int>(), tr("Before"));
    CT_OutStandardItemDrawableModel *itemOutModel_theo = new CT_OutStandardItemDrawableModel(DEF_itemOut_theo, new CT_Grid3D<int>(), tr("Theorical"));
    CT_OutStandardItemDrawableModel *itemOutModel_hits = new CT_OutStandardItemDrawableModel(DEF_itemOut_hits, new CT_Grid3D<int>(), tr("Hits"));
    CT_OutStandardItemDrawableModel *itemOutModel_density = new CT_OutStandardItemDrawableModel(DEF_itemOut_density, new CT_Grid3D<double>(), tr("Density"));
    CT_OutStandardItemDrawableModel *itemOutModel_scan = new CT_OutStandardItemDrawableModel(DEF_itemOut_scan, new CT_Scanner(), tr("Scanner"));

    groupOutModel_base->addItem(itemOutModel_deltaout);
    groupOutModel_base->addItem(itemOutModel_deltain);
    groupOutModel_base->addItem(itemOutModel_deltabef);
    groupOutModel_base->addItem(itemOutModel_deltatheo);
    groupOutModel_base->addItem(itemOutModel_bef);
    groupOutModel_base->addItem(itemOutModel_theo);
    groupOutModel_base->addItem(itemOutModel_hits);
    groupOutModel_base->addItem(itemOutModel_density);
    groupOutModel_base->addItem(itemOutModel_scan);

    CT_OutResultModelGroup *resultOutModel_grid = new CT_OutResultModelGroup(DEF_resultOut_grid, groupOutModel_base, tr("grids"), tr("Computed grids"));
    addOutResultModel(resultOutModel_grid);
}

void LVOX_StepComputeDensity::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    //********************************************//
    //              Attributes of LVox            //
    //********************************************//
    configDialog->addDouble(tr("Size of a voxel"),tr("meters"),0.0001,10000,2, _res );
    configDialog->addDouble(tr("Minimum intensity taken in account"),tr(""),-100000,100000,2, _intensityThresh );
    configDialog->addBool("Consider the threshold as a minimum value","","",_greaterThanThresh);
    configDialog->addDouble(tr("Minimum number of effective ray in a voxel to take it into account"),tr(""),-100000,100000,2, _effectiveRayThresh );

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

void LVOX_StepComputeDensity::compute()
{
    // Get the input scene parameter
    CT_ResultGroup *inResult = getInputResults().first();
    CT_InAbstractItemDrawableModel *inSceneModel = (CT_InAbstractItemDrawableModel*) getInModelForResearch(inResult, DEF_SearchInScene);
    inResult->recursiveBeginIterateItems(*inSceneModel);
    const CT_Scene* scene = (CT_Scene*) inResult->recursiveNextItem();

    if (scene==NULL) {return;}

    // Gets the out result
    CT_ResultGroup* outResult = getOutResultList().first();

    // Get the base group model and create base group
    CT_OutStandardGroupModel* groupOutModel_base = (CT_OutStandardGroupModel*)getOutModelForCreation(outResult, DEF_groupOut_base);
    CT_StandardItemGroup* baseGroup = new CT_StandardItemGroup(groupOutModel_base,outResult);
    outResult->addGroup(baseGroup);

    // Get the items models
    CT_OutStandardItemDrawableModel* itemOutModel_deltaout = (CT_OutStandardItemDrawableModel*)getOutModelForCreation(outResult, DEF_itemOut_deltaout);
    CT_OutStandardItemDrawableModel* itemOutModel_deltain = (CT_OutStandardItemDrawableModel*)getOutModelForCreation(outResult, DEF_itemOut_deltain);
    CT_OutStandardItemDrawableModel* itemOutModel_deltabef = (CT_OutStandardItemDrawableModel*)getOutModelForCreation(outResult, DEF_itemOut_deltabef);
    CT_OutStandardItemDrawableModel* itemOutModel_deltatheo = (CT_OutStandardItemDrawableModel*)getOutModelForCreation(outResult, DEF_itemOut_deltatheo);
    CT_OutStandardItemDrawableModel* itemOutModel_bef = (CT_OutStandardItemDrawableModel*)getOutModelForCreation(outResult, DEF_itemOut_bef);
    CT_OutStandardItemDrawableModel* itemOutModel_theo = (CT_OutStandardItemDrawableModel*)getOutModelForCreation(outResult, DEF_itemOut_theo);
    CT_OutStandardItemDrawableModel* itemOutModel_hits = (CT_OutStandardItemDrawableModel*)getOutModelForCreation(outResult, DEF_itemOut_hits);
    CT_OutStandardItemDrawableModel* itemOutModel_density = (CT_OutStandardItemDrawableModel*)getOutModelForCreation(outResult, DEF_itemOut_density);
    CT_OutStandardItemDrawableModel* itemOutModel_scan = (CT_OutStandardItemDrawableModel*)getOutModelForCreation(outResult, DEF_itemOut_scan);

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
    CT_Grid3D<double>*   deltaInGrid = new CT_Grid3D<double>(itemOutModel_deltain, outResult, hitGrid->xMin(), hitGrid->yMin(), hitGrid->zMin(), hitGrid->xdim(), hitGrid->ydim(), hitGrid->zdim(), _res, -1, 0);
    CT_Grid3D<double>*   deltaOutGrid = new CT_Grid3D<double>(itemOutModel_deltaout, outResult, hitGrid->xMin(), hitGrid->yMin(), hitGrid->zMin(), hitGrid->xdim(), hitGrid->ydim(), hitGrid->zdim(), _res, -1, 0);
    CT_Grid3D<int>*      theoriticalGrid = new CT_Grid3D<int>(itemOutModel_theo, outResult, hitGrid->xMin(), hitGrid->yMin(), hitGrid->zMin(), hitGrid->xdim(), hitGrid->ydim(), hitGrid->zdim(), _res, -1, 0);
    CT_Grid3D<double>*   deltaTheoritical = new CT_Grid3D<double>(itemOutModel_deltatheo, outResult, hitGrid->xMin(), hitGrid->yMin(), hitGrid->zMin(), hitGrid->xdim(), hitGrid->ydim(), hitGrid->zdim(), _res, -1, 0);
    CT_Grid3D<int>*      beforeGrid = new CT_Grid3D<int>(itemOutModel_bef, outResult, hitGrid->xMin(), hitGrid->yMin(), hitGrid->zMin(), hitGrid->xdim(), hitGrid->ydim(), hitGrid->zdim(), _res, -1, 0);
    CT_Grid3D<double>*   deltaBefore = new CT_Grid3D<double>(itemOutModel_deltabef, outResult, hitGrid->xMin(), hitGrid->yMin(), hitGrid->zMin(), hitGrid->xdim(), hitGrid->ydim(), hitGrid->zdim(), _res, -1, 0);
    CT_Grid3D<double>*   densityGrid = new CT_Grid3D<double>(itemOutModel_density, outResult, hitGrid->xMin(), hitGrid->yMin(), hitGrid->zMin(), hitGrid->xdim(), hitGrid->ydim(), hitGrid->zdim(), _res, -1, 0);

    setProgress(5);

    LVOX_ComputeHitsThread hitsThread(scanner, hitGrid, deltaInGrid, deltaOutGrid, scene, _intensityThresh, _greaterThanThresh);
    hitsThread.start();

    LVOX_ComputeTheoriticalsThread theoricalThread(scanner, theoriticalGrid, deltaTheoritical);
    theoricalThread.start();


    LVOX_ComputeBeforeThread beforeThread(scanner, beforeGrid, deltaBefore, scene, _intensityThresh, _greaterThanThresh);
    beforeThread.start();


    //    computeGridTools::computeDensityGrid(densityGrid, hitGrid, theoriticalGrid, beforeGrid, _effectiveRayThresh );

    hitsThread.wait();
    beforeThread.wait();
    theoricalThread.wait();

    baseGroup->addItemDrawable(deltaOutGrid);
    baseGroup->addItemDrawable(deltaInGrid);
    baseGroup->addItemDrawable(deltaBefore);
    baseGroup->addItemDrawable(deltaTheoritical);
    baseGroup->addItemDrawable(beforeGrid);
    baseGroup->addItemDrawable(theoriticalGrid);
    baseGroup->addItemDrawable(hitGrid);
    baseGroup->addItemDrawable(densityGrid);
    
    setProgress(100);

}
