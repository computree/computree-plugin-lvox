/****************************************************************************

 Copyright (C) 2010-2016 the Office National des Forêts (ONF), France
                         All rights reserved.

 Contact : alexandre.piboule@onf.fr

 Developers : Alexandre PIBOULE (ONF)

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

#include "lvox_stepcomputeocclusionspace.h"

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

#include "tools/lvox_computebeforewithlengththresholdthread.h"

#include <QFileInfo>
#include <QDebug>
#include <limits>


#define DEF_SearchInResult "r"
#define DEF_SearchInScene   "sc"
#define DEF_SearchInScan   "sca"
#define DEF_SearchInGroup   "gr"

LVOX_StepComputeOcclusionsSpace::LVOX_StepComputeOcclusionsSpace(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    //********************************************//
    //              Attributes of LVox            //
    //********************************************//
    _res = 0.5;
    _distThreshold = 1.0;
    _gridMode = 1;
    _xBase = -20.0;
    _yBase = -20.0;
    _zBase = -10.0;

    _xDim = 80;
    _yDim = 80;
    _zDim = 80;
}

QString LVOX_StepComputeOcclusionsSpace::getStepDescription() const
{
    // Gives the descrption to print in the GUI
    return tr("Compute occlusions space");
}

CT_VirtualAbstractStep* LVOX_StepComputeOcclusionsSpace::createNewInstance(CT_StepInitializeData &dataInit)
{
    // Creates an instance of this step
    return new LVOX_StepComputeOcclusionsSpace(dataInit);
}

void LVOX_StepComputeOcclusionsSpace::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy *resultModel = createNewInResultModelForCopy(DEF_SearchInResult, tr("Scène(s)"));

    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_SearchInGroup, CT_AbstractItemGroup::staticGetType(), tr("Scan"));
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInScene, CT_Scene::staticGetType(), tr("Scène"));
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInScan, CT_Scanner::staticGetType(), tr("Scanner"));

}

void LVOX_StepComputeOcclusionsSpace::createOutResultModelListProtected()
{
    // create a new OUT result that is a copy of the IN result selected by the user
    CT_OutResultModelGroupToCopyPossibilities *res = createNewOutResultModelToCopy(DEF_SearchInResult);

    if (res != NULL)
    {
        // in this result we add a CT_AffiliationID to the group named DEF_SearchInGroup. The name of the model of
        // the CT_AffiliationID will be generated automatically by the _outAffiliationIDModelName object.
        res->addItemModel(DEF_SearchInGroup, _occl_ModelName, new CT_Grid3D<int>(), tr("Occlusion space"));
    }

}

void LVOX_StepComputeOcclusionsSpace::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    //********************************************//
    //              Attributes of LVox            //
    //********************************************//
    configDialog->addDouble(tr("Resolution of the grids"),tr("meters"),0.0001,10000,2, _res );
    configDialog->addDouble(tr("Distance Threshold"), "m", 0, std::numeric_limits<float>::max(), 4, _distThreshold);

    configDialog->addEmpty();

    configDialog->addText(tr("Reference for (minX, minY, minZ) corner of the grid :"),"", "");

    CT_ButtonGroup &bg_gridMode = configDialog->addButtonGroup(_gridMode);
    configDialog->addExcludeValue("", "", tr("Default mode : Bounding box of the scene"), bg_gridMode, 0);
    configDialog->addExcludeValue("", "", tr("Custom mode : Relative to folowing coordinates:"), bg_gridMode, 1);
    configDialog->addExcludeValue("", "", tr("Custom mode : Relative to folowing coordinates + custom dimensions:"), bg_gridMode, 2);
    configDialog->addExcludeValue("", "", tr("Custom mode : centered on folowing coordinates + custom dimensions:"), bg_gridMode, 3);


    configDialog->addDouble(tr("X coordinate:"), "", -std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), 4, _xBase);
    configDialog->addDouble(tr("Y coordinate:"), "", -std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), 4, _yBase);
    configDialog->addDouble(tr("Z coordinate:"), "", -std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), 4, _zBase);

    configDialog->addInt(tr("X dimension:"), "", 1, 1000, _xDim);
    configDialog->addInt(tr("Y dimension:"), "", 1, 1000, _yDim);
    configDialog->addInt(tr("Z dimension:"), "", 1, 1000, _zDim);


}

void LVOX_StepComputeOcclusionsSpace::compute()
{   
    // Gets the out result
    CT_ResultGroup* outResult = getOutResultList().first();

    QList<CT_MonitoredQThread*> baseThreads;

    QMap<CT_AbstractItemGroup*, QPair<const CT_Scene*, const CT_Scanner*> > pointsOfView;

    // Global limits of generated grids
    double xMin = std::numeric_limits<double>::max();
    double yMin = std::numeric_limits<double>::max();
    double zMin = std::numeric_limits<double>::max();
    double xMax = -std::numeric_limits<double>::max();
    double yMax = -std::numeric_limits<double>::max();
    double zMax = -std::numeric_limits<double>::max();

    double xMinScene = std::numeric_limits<double>::max();
    double yMinScene = std::numeric_limits<double>::max();
    double zMinScene = std::numeric_limits<double>::max();
    double xMaxScene = -std::numeric_limits<double>::max();
    double yMaxScene = -std::numeric_limits<double>::max();
    double zMaxScene = -std::numeric_limits<double>::max();

    double xMinScanner = std::numeric_limits<double>::max();
    double yMinScanner = std::numeric_limits<double>::max();
    double zMinScanner = std::numeric_limits<double>::max();
    double xMaxScanner = -std::numeric_limits<double>::max();
    double yMaxScanner = -std::numeric_limits<double>::max();
    double zMaxScanner = -std::numeric_limits<double>::max();



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

            Eigen::Vector3d min, max;
            scene->getBoundingBox(min, max);

            if (min.x() < xMinScene) {xMinScene = min.x();}
            if (min.y() < yMinScene) {yMinScene = min.y();}
            if (min.z() < zMinScene) {zMinScene = min.z();}
            if (max.x() > xMaxScene) {xMaxScene = max.x();}
            if (max.y() > yMaxScene) {yMaxScene = max.y();}
            if (max.z() > zMaxScene) {zMaxScene = max.z();}

            if (scanner->getCenterX() < xMinScanner) {xMinScanner = scanner->getCenterX();}
            if (scanner->getCenterY() < yMinScanner) {yMinScanner = scanner->getCenterY();}
            if (scanner->getCenterZ() < zMinScanner) {zMinScanner = scanner->getCenterZ();}

            if (scanner->getCenterX() > xMaxScanner) {xMaxScanner = scanner->getCenterX();}
            if (scanner->getCenterY() > yMaxScanner) {yMaxScanner = scanner->getCenterY();}
            if (scanner->getCenterZ() > zMaxScanner) {zMaxScanner = scanner->getCenterZ();}
        }
    }

    if (_gridMode == 0) {

        xMin = std::min(xMinScene, xMinScanner);
        yMin = std::min(yMinScene, yMinScanner);
        zMin = std::min(zMinScene, zMinScanner);

        xMax = std::max(xMaxScene, xMaxScanner);
        yMax = std::max(yMaxScene, yMaxScanner);
        zMax = std::max(zMaxScene, zMaxScanner);

    } else if (_gridMode == 1) {

        double xMinAdjusted = std::min(xMinScene, xMinScanner);
        double yMinAdjusted = std::min(yMinScene, yMinScanner);
        double zMinAdjusted = std::min(zMinScene, zMinScanner);

        double xMaxAdjusted = std::max(xMaxScene, xMaxScanner);
        double yMaxAdjusted = std::max(yMaxScene, yMaxScanner);
        double zMaxAdjusted = std::max(zMaxScene, zMaxScanner);

        xMin = _xBase;
        yMin = _yBase;
        zMin = _zBase;

        while (xMin < xMinAdjusted) {xMin += _res;}
        while (yMin < yMinAdjusted) {yMin += _res;}
        while (zMin < zMinAdjusted) {zMin += _res;}

        while (xMin > xMinAdjusted) {xMin -= _res;}
        while (yMin > yMinAdjusted) {yMin -= _res;}
        while (zMin > zMinAdjusted) {zMin -= _res;}

        xMax = xMin + _res;
        yMax = yMin + _res;
        zMax = zMin + _res;

        while (xMax < xMaxAdjusted) {xMax += _res;}
        while (yMax < yMaxAdjusted) {yMax += _res;}
        while (zMax < zMaxAdjusted) {zMax += _res;}

    } else if (_gridMode == 2) {

        xMin = _xBase;
        yMin = _yBase;
        zMin = _zBase;

        xMax = xMin + _res*_xDim;
        yMax = yMin + _res*_yDim;
        zMax = zMin + _res*_zDim;

        bool enlarged = false;

        while (xMin > xMinScanner) {xMin -= _res; enlarged = true;}
        while (yMin > yMinScanner) {yMin -= _res; enlarged = true;}
        while (zMin > zMinScanner) {zMin -= _res; enlarged = true;}

        while (xMax < xMaxScanner) {xMax += _res; enlarged = true;}
        while (yMax < yMaxScanner) {yMax += _res; enlarged = true;}
        while (zMax < zMaxScanner) {zMax += _res; enlarged = true;}

        if (enlarged)
        {
            PS_LOG->addMessage(LogInterface::warning, LogInterface::step, tr("Dimensions spécifiées ne contenant pas les positions de scans : la grille a du être élargie !"));
        }
    } else {

        xMin = _xBase - _res*_xDim;
        yMin = _yBase - _res*_yDim;
        zMin = _zBase - _res*_zDim;

        xMax = _xBase + _res*_xDim;
        yMax = _yBase + _res*_yDim;
        zMax = _zBase + _res*_zDim;

        bool enlarged = false;

        while (xMin > xMinScanner) {xMin -= _res; enlarged = true;}
        while (yMin > yMinScanner) {yMin -= _res; enlarged = true;}
        while (zMin > zMinScanner) {zMin -= _res; enlarged = true;}

        while (xMax < xMaxScanner) {xMax += _res; enlarged = true;}
        while (yMax < yMaxScanner) {yMax += _res; enlarged = true;}
        while (zMax < zMaxScanner) {zMax += _res; enlarged = true;}

        if (enlarged)
        {
            PS_LOG->addMessage(LogInterface::warning, LogInterface::step, tr("Dimensions spécifiées ne contenant pas les positions de scans : la grille a du être élargie !"));
        }
    }

    xMin -= _res;
    yMin -= _res;
    zMin -= _res;
    xMax += _res;
    yMax += _res;
    zMax += _res;

    QMapIterator<CT_AbstractItemGroup*, QPair<const CT_Scene*, const CT_Scanner*> > it(pointsOfView);
    while (it.hasNext() && !isStopped())
    {
        it.next();
        CT_AbstractItemGroup* group = it.key();
        const CT_Scene* scene = it.value().first;
        const CT_Scanner* scanner =it.value().second;

        // Declaring the output grids
        CT_Grid3D<int>*      occlGrid = CT_Grid3D<int>::createGrid3DFromXYZCoords(_occl_ModelName.completeName(), outResult, xMin, yMin, zMin, xMax, yMax, zMax, _res, -1, 0, true);

        group->addItemDrawable(occlGrid);

        LVOX_ComputeBeforeWithLengthThresholdThread* occlThread = new LVOX_ComputeBeforeWithLengthThresholdThread(scanner, occlGrid, scene, _distThreshold);
        connect(occlThread, SIGNAL(progressChanged()), this, SLOT(updateProgress()));
        _threadList.append(occlThread);
        baseThreads.append(occlThread);
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

    qDeleteAll(_threadList);

    setProgress(100);
}

void LVOX_StepComputeOcclusionsSpace::updateProgress()
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

