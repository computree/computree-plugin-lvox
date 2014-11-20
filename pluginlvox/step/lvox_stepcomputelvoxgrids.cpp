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

#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"

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

    _gridMode = 1;
    _xBase = 0.0;
    _yBase = 0.0;
    _zBase = 0.0;

    _xDim = 0;
    _yDim = 0;
    _zDim = 0;
}

QString LVOX_StepComputeLvoxGrids::getStepDescription() const
{
    // Gives the descrption to print in the GUI
    return tr("Calcul de grilles 3D densités corrigées");
}

CT_VirtualAbstractStep* LVOX_StepComputeLvoxGrids::createNewInstance(CT_StepInitializeData &dataInit)
{
    // Creates an instance of this step
    return new LVOX_StepComputeLvoxGrids(dataInit);
}

void LVOX_StepComputeLvoxGrids::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy *resultModel = createNewInResultModelForCopy(DEF_SearchInResult, tr("Scène(s)"));

    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_SearchInGroup, CT_AbstractItemGroup::staticGetType(), tr("Scan"));
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInScene, CT_Scene::staticGetType(), tr("Scène"));
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInScan, CT_Scanner::staticGetType(), tr("Scanner"));

}

void LVOX_StepComputeLvoxGrids::createOutResultModelListProtected()
{
    // create a new OUT result that is a copy of the IN result selected by the user
    CT_OutResultModelGroupToCopyPossibilities *res = createNewOutResultModelToCopy(DEF_SearchInResult);

    // in this result we add a CT_AffiliationID to the group named DEF_SearchInGroup. The name of the model of
    // the CT_AffiliationID will be generated automatically by the _outAffiliationIDModelName object.
    res->addItemModel(DEF_SearchInGroup, _hits_ModelName, new CT_Grid3D<int>(), tr("Hits"));
    res->addItemModel(DEF_SearchInGroup, _theo_ModelName, new CT_Grid3D<int>(), tr("Theoretical"));
    res->addItemModel(DEF_SearchInGroup, _bef_ModelName, new CT_Grid3D<int>(), tr("Before"));
    res->addItemModel(DEF_SearchInGroup, _density_ModelName, new CT_Grid3D<float>(), tr("Density"));

    if (_computeDistances)
    {
        res->addItemModel(DEF_SearchInGroup, _deltain_ModelName, new CT_Grid3D<float>(), tr("DeltaIn"));
        res->addItemModel(DEF_SearchInGroup, _deltaout_ModelName, new CT_Grid3D<float>(), tr("DeltaOut"));
        res->addItemModel(DEF_SearchInGroup, _deltatheo_ModelName, new CT_Grid3D<float>(), tr("Deltatheoretical"));
        res->addItemModel(DEF_SearchInGroup, _deltabef_ModelName, new CT_Grid3D<float>(), tr("DeltaBefore"));

    }
}

void LVOX_StepComputeLvoxGrids::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    //********************************************//
    //              Attributes of LVox            //
    //********************************************//
    configDialog->addDouble(tr("Resolution of the grids"),tr("meters"),0.0001,10000,2, _res );
    configDialog->addInt(tr("Minimum number of effective ray in a voxel to take it into account"),tr(""),0,100000, _effectiveRayThresh );
    configDialog->addBool(tr("Compute Distances"), tr(""), tr(""), _computeDistances);

    configDialog->addEmpty();

    configDialog->addText(tr("Reference for (minX, minY, minZ) corner of the grid :"),"", "");

    CT_ButtonGroup &bg_gridMode = configDialog->addButtonGroup(_gridMode);
    configDialog->addExcludeValue("", "", tr("Default mode : Bounding box of the scene"), bg_gridMode, 0);
    configDialog->addExcludeValue("", "", tr("Custom mode : Relative to folowing coordinates:"), bg_gridMode, 1);
    configDialog->addExcludeValue("", "", tr("Custom mode : Relative to folowing coordinates + custom dimensions:"), bg_gridMode, 2);


    configDialog->addDouble(tr("X coordinate:"), "", -std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), 4, _xBase);
    configDialog->addDouble(tr("Y coordinate:"), "", -std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), 4, _yBase);
    configDialog->addDouble(tr("Z coordinate:"), "", -std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), 4, _zBase);

    configDialog->addInt(tr("X dimension:"), "", 1, 1000, _xDim);
    configDialog->addInt(tr("Y dimension:"), "", 1, 1000, _yDim);
    configDialog->addInt(tr("Z dimension:"), "", 1, 1000, _zDim);


}

void LVOX_StepComputeLvoxGrids::compute()
{   
    // Gets the out result
    CT_ResultGroup* outResult = getOutResultList().first();

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
    CT_ResultGroupIterator itGrp(outResult, this, DEF_SearchInGroup);
    while (itGrp.hasNext() && !isStopped())
    {
        CT_AbstractItemGroup *group = (CT_AbstractItemGroup*) itGrp.next();

        const CT_Scene* scene = (CT_Scene*)group->firstItemByINModelName(this, DEF_SearchInScene);
        const CT_Scanner* scanner = (CT_Scanner*)group->firstItemByINModelName(this, DEF_SearchInScan);

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

    if (_gridMode == 1 || _gridMode == 2)
    {
        float xMinScenes = xMin;
        float yMinScenes = yMin;
        float zMinScenes = zMin;

        xMin = _xBase;
        yMin = _yBase;
        zMin = _zBase;

        while (xMin < xMinScenes) {xMin += _res;};
        while (yMin < yMinScenes) {yMin += _res;};
        while (zMin < zMinScenes) {zMin += _res;};

        while (xMin > xMinScenes) {xMin -= _res;};
        while (yMin > yMinScenes) {yMin -= _res;};
        while (zMin > zMinScenes) {zMin -= _res;};

        if( _gridMode == 2){
            xMax = xMin + _res*_xDim;
            yMax = yMin + _res*_yDim;
            zMax = zMin + _res*_zDim;
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
        CT_Grid3D<int>*      hitGrid = CT_Grid3D<int>::createGrid3DFromXYZCoords(_hits_ModelName.completeName(), outResult, xMin, yMin, zMin, xMax, yMax, zMax, _res, -1, 0);
        CT_Grid3D<int>*      theoriticalGrid = new CT_Grid3D<int>(_theo_ModelName.completeName(), outResult, hitGrid->minX(), hitGrid->minY(), hitGrid->minZ(), hitGrid->xdim(), hitGrid->ydim(), hitGrid->zdim(), _res, -1, 0);
        CT_Grid3D<int>*      beforeGrid = new CT_Grid3D<int>(_bef_ModelName.completeName(), outResult, hitGrid->minX(), hitGrid->minY(), hitGrid->minZ(), hitGrid->xdim(), hitGrid->ydim(), hitGrid->zdim(), _res, -1, 0);
        CT_Grid3D<float>*   density = new CT_Grid3D<float>(_density_ModelName.completeName(), outResult, hitGrid->minX(), hitGrid->minY(), hitGrid->minZ(), hitGrid->xdim(), hitGrid->ydim(), hitGrid->zdim(), _res, -1, 0);

        CT_Grid3D<float>*   deltaInGrid = NULL;
        CT_Grid3D<float>*   deltaOutGrid = NULL;
        CT_Grid3D<float>*   deltaTheoritical = NULL;
        CT_Grid3D<float>*   deltaBefore = NULL;

        group->addItemDrawable(hitGrid);
        group->addItemDrawable(theoriticalGrid);
        group->addItemDrawable(beforeGrid);
        group->addItemDrawable(density);

        if (_computeDistances)
        {
            deltaInGrid = new CT_Grid3D<float>(_deltain_ModelName.completeName(), outResult, hitGrid->minX(), hitGrid->minY(), hitGrid->minZ(), hitGrid->xdim(), hitGrid->ydim(), hitGrid->zdim(), _res, -1, 0);
            deltaOutGrid = new CT_Grid3D<float>(_deltaout_ModelName.completeName(), outResult, hitGrid->minX(), hitGrid->minY(), hitGrid->minZ(), hitGrid->xdim(), hitGrid->ydim(), hitGrid->zdim(), _res, -1, 0);
            deltaTheoritical = new CT_Grid3D<float>(_deltatheo_ModelName.completeName(), outResult, hitGrid->minX(), hitGrid->minY(), hitGrid->minZ(), hitGrid->xdim(), hitGrid->ydim(), hitGrid->zdim(), _res, -1, 0);
            deltaBefore = new CT_Grid3D<float>(_deltabef_ModelName.completeName(), outResult, hitGrid->minX(), hitGrid->minY(), hitGrid->minZ(), hitGrid->xdim(), hitGrid->ydim(), hitGrid->zdim(), _res, -1, 0);

            group->addItemDrawable(deltaInGrid);
            group->addItemDrawable(deltaOutGrid);
            group->addItemDrawable(deltaTheoritical);
            group->addItemDrawable(deltaBefore);
        }

        LVOX_ComputeHitsThread* hitsThread = new LVOX_ComputeHitsThread(scanner, hitGrid, deltaInGrid, deltaOutGrid, scene, _computeDistances);
        connect(hitsThread, SIGNAL(progressChanged()), this, SLOT(updateProgress()));
        _threadList.append(hitsThread);
        baseThreads.append(hitsThread);

        LVOX_ComputeTheoriticalsThread* theoreticalThread = new LVOX_ComputeTheoriticalsThread(scanner, theoriticalGrid, deltaTheoritical, _computeDistances);
        connect(theoreticalThread, SIGNAL(progressChanged()), this, SLOT(updateProgress()));
        _threadList.append(theoreticalThread);
        baseThreads.append(theoreticalThread);

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
        updateProgress();
    }

    for (int i = 0 ; i < size ; ++i)
    {
        disconnect(baseThreads.at(i), SIGNAL(progressChanged()), this, SLOT(updateProgress()));
    }

    size = densityThreads.size();

    for (int i = 0 ; i < size ; ++i)
    {
        densityThreads.at(i)->start();

    }

    for (int i = 0 ; i < size ; ++i)
    {
        densityThreads.at(i)->wait();
        updateProgress();
    }

    for (int i = 0 ; i < size ; ++i)
    {
        disconnect(densityThreads.at(i), SIGNAL(progressChanged()), this, SLOT(updateProgress()));
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

