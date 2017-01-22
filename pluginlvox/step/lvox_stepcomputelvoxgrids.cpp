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
#include "ct_itemdrawable/CT_FileHeader.h"
#include "qvector3d.h"

#include "ct_step/abstract/ct_abstractsteploadfile.h"
#include "ct_view/ct_stepconfigurabledialog.h"

#ifdef CT_LARCHIHEADER_H
#include "ctlibio/readers/ct_reader_larchitect_grid.h"
#endif

#include "tools/lvox_computehitsthread.h"
#include "tools/lvox_computetheoriticalsthread.h"
#include "tools/lvox_computebeforethread.h"
#include "tools/lvox_computedensitythread.h"

#include <QFileInfo>
#include <QDebug>
#include <QMap>
#include <limits>


#include "qfile.h"
#include "qtextstream.h"
#include "qfileinfo.h"
#include "qdebug.h"



#define DEF_SearchInResult "r"
#define DEF_SearchInScene   "sc"
#define DEF_SearchInScan   "sca"
#define DEF_SearchInGroup   "gr"

//Alias for in model
#define DEF_resultIn_header "rheader"
#define DEF_groupIn_header "gheader"
#define DEF_inATTisNi "isNi"
#define DEF_itemIn_header "header"


#define DEF_CT_header "genericHeader"




LVOX_StepComputeLvoxGrids::LVOX_StepComputeLvoxGrids(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    //********************************************//
    //              Attributes of LVox            //
    //********************************************//
    _res = 0.5;
    _effectiveRayThresh = 10;
    _computeDistances = false;

    _gridMode = 0;
    _xBase = -20.0;
    _yBase = -20.0;
    _zBase = -10.0;

    _xDim = 80;
    _yDim = 80;
    _zDim = 80;
}

QString LVOX_StepComputeLvoxGrids::getStepDescription() const
{
    // Gives the descrption to print in the GUI
    return tr("2- Calculer les grilles LVOX");
}

CT_VirtualAbstractStep* LVOX_StepComputeLvoxGrids::createNewInstance(CT_StepInitializeData &dataInit)
{
    // Creates an instance of this step
    return new LVOX_StepComputeLvoxGrids(dataInit);
}

void LVOX_StepComputeLvoxGrids::createInResultModelListProtected()
{
//    CT_InResultModelGroup *resultLARCHI = createNewInResultModel(DEF_resultIn_header, tr("Résultat d'en-tête"));
//   resultLARCHI->setZeroOrMoreRootGroup();
//    resultLARCHI->addGroupModel("", DEF_groupIn_header, CT_AbstractItemGroup::staticGetType(), tr("Groupe Header"));


    CT_InResultModelGroupToCopy *resultScan = createNewInResultModelForCopy(DEF_SearchInResult, tr("Scène"), "", true);
    resultScan->setZeroOrMoreRootGroup();
    resultScan->addGroupModel("", DEF_SearchInGroup, CT_AbstractItemGroup::staticGetType(), tr("Scan"));
    resultScan->addItemModel(DEF_SearchInGroup, DEF_SearchInScene, CT_Scene::staticGetType(), tr("Scène"));
    resultScan->addItemModel(DEF_SearchInGroup, DEF_SearchInScan, CT_Scanner::staticGetType(), tr("Scanner"));


/* Tentative d'ajout d'un résultat d'entrée de type "header de fichier". Ceci avait pour but de récupérer facilement les coordonnées d'une grille
 * de référence pour l'utiliser comme base au calcul de nouvelles grilles de voxels. Sans succès. */
/* Finalement ceci a été géré par un nouveau lecteur de header spécifique aux grilles L-Architect : CT_LARCHIHEADER_H                     */
#ifdef CT_LARCHIHEADER_H
//    resultScan->addItemModel(DEF_SearchInGroup, DEF_itemIn_header, CT_FileHeader::staticGetType(), tr("Grille de référence"), "",
//                               CT_InAbstractModel::C_ChooseOneIfMultiple, CT_InAbstractModel::F_IsOptional);


//    resultModel->addItemModel(DEF_groupIn_grids, DEF_itemIn_hits, CT_Grid3D<int>::staticGetType(), tr("hits"), "",
//                              CT_InAbstractModel::C_ChooseOneIfMultiple, CT_InAbstractModel::F_IsOptional);
//    resultModel->addItemAttributeModel(DEF_itemIn_hits, DEF_inATTisNi, QList<QString>() << "LVOX_GRD_NI", CT_AbstractCategory::ANY, tr("isNi"), "", CT_InAbstractModel::C_ChooseOneIfMultiple, CT_InAbstractModel::F_IsOptional);
#endif

}

void LVOX_StepComputeLvoxGrids::createOutResultModelListProtected()
{
    // create a new OUT result that is a copy of the IN result selected by the user
    CT_OutResultModelGroupToCopyPossibilities *res = createNewOutResultModelToCopy(DEF_SearchInResult);

    if (res != NULL)
    {
        // in this result we add a CT_AffiliationID to the group named DEF_SearchInGroup. The name of the model of
        // the CT_AffiliationID will be generated automatically by the _outAffiliationIDModelName object.
        res->addItemModel(DEF_SearchInGroup, _hits_ModelName, new CT_Grid3D<int>(), tr("Hits"));
        res->addItemAttributeModel(_hits_ModelName, _NiFlag_ModelName, new CT_StdItemAttributeT<bool>("LVOX_GRD_NI"), tr("isNi"));

        res->addItemModel(DEF_SearchInGroup, _theo_ModelName, new CT_Grid3D<int>(), tr("Theoretical"));
        res->addItemAttributeModel(_theo_ModelName, _NtFlag_ModelName, new CT_StdItemAttributeT<bool>("LVOX_GRD_NT"), tr("isNt"));

        res->addItemModel(DEF_SearchInGroup, _bef_ModelName, new CT_Grid3D<int>(), tr("Before"));
        res->addItemAttributeModel(_bef_ModelName, _NbFlag_ModelName, new CT_StdItemAttributeT<bool>("LVOX_GRD_NB"), tr("isNb"));

        res->addItemModel(DEF_SearchInGroup, _density_ModelName, new CT_Grid3D<float>(), tr("Density"));
        res->addItemAttributeModel(_density_ModelName, _DensityFlag_ModelName, new CT_StdItemAttributeT<bool>("LVOX_GRD_DENSITY"), tr("isDensity"));


        if (_computeDistances)
        {
            res->addItemModel(DEF_SearchInGroup, _deltain_ModelName, new CT_Grid3D<float>(), tr("DeltaIn"));
            res->addItemModel(DEF_SearchInGroup, _deltaout_ModelName, new CT_Grid3D<float>(), tr("DeltaOut"));
            res->addItemModel(DEF_SearchInGroup, _deltatheo_ModelName, new CT_Grid3D<float>(), tr("Deltatheoretical"));
            res->addItemModel(DEF_SearchInGroup, _deltabef_ModelName, new CT_Grid3D<float>(), tr("DeltaBefore"));

        }
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
    configDialog->addExcludeValue("", "", tr("Custom mode : centered on folowing coordinates + custom dimensions:"), bg_gridMode, 3);
#ifdef CT_LARCHIHEADER_H
    configDialog->addExcludeValue("", "", tr("Automatic mode : use grid paramters from grid file"), bg_gridMode, 4);
    configDialog->addFileChoice(tr("Choisir le fichier .grid"), CT_FileChoiceButton::OneExistingFile, "Fichier L-Architect (*.grid)", _gridfileName);
#endif

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



    if (_gridMode == 0) {   // Boite englobante = simple
        PS_LOG->addInfoMessage(LogInterface::trace, "Mode " + QString::number(_gridMode));

        xMin = std::min(xMinScene, xMinScanner);
        yMin = std::min(yMinScene, yMinScanner);
        zMin = std::min(zMinScene, zMinScanner);

        xMax = std::max(xMaxScene, xMaxScanner);
        yMax = std::max(yMaxScene, yMaxScanner);
        zMax = std::max(zMaxScene, zMaxScanner);

    } else if (_gridMode == 1) { //
        PS_LOG->addInfoMessage(LogInterface::trace, "Mode " + QString::number(_gridMode));

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
        PS_LOG->addInfoMessage(LogInterface::trace, "Mode " + QString::number(_gridMode));

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
    } else if (_gridMode == 3)   {
        PS_LOG->addInfoMessage(LogInterface::trace, "Mode " + QString::number(_gridMode));

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
    } else if (_gridMode == 4){

#ifdef CT_LARCHIHEADER_H

        PS_LOG->addInfoMessage(LogInterface::trace, "Mode " + QString::number(_gridMode));

            QFileInfo fileInfo(_gridfileName.first());

            if (fileInfo.exists())
            {
                QString extension = fileInfo.completeSuffix();
                CT_Reader_LArchitect_Grid *reader;

                if(extension == "grid"){
                    reader = new CT_Reader_LArchitect_Grid();
                    PS_LOG->addMessage(LogInterface::trace, LogInterface::step, QObject::tr("File reader created"));

                    if (reader->setFilePath(_gridfileName.first()) != NULL)
                    {
                        reader->init();

                        CT_LARCHIHEADER * _header = reader->getHeader();

                        PS_LOG->addInfoMessage(LogInterface::step, "  : DIM XYZ [" + QString::number(_header->get_xDim()) + ";" + QString::number(_header->get_yDim()) + ";" + QString::number(_header->get_zDim()) + "] ");
                        PS_LOG->addInfoMessage(LogInterface::step, "  : RES XYZ [" + QString::number(_header->get_Res(0)) + ";" + QString::number(_header->get_Res(1)) + ";" + QString::number(_header->get_Res(2)) + "] ");
                        PS_LOG->addInfoMessage(LogInterface::step, "  : MIN XYZ [" + QString::number(_header->get_xMin()) + ";" + QString::number(_header->get_yMin()) + ";" + QString::number(_header->get_zMin()) + "] ");

                        xMin = _header->get_xMin();
                        yMin = _header->get_yMin();
                        zMin = _header->get_zMin();

                        _xDim = _header->get_xDim()*_header->get_Res(0)/_res;
                        _yDim = _header->get_yDim()*_header->get_Res(1)/_res;
                        _zDim = _header->get_zDim()*_header->get_Res(2)/_res;

                        xMax = xMin + _header->get_xDim()*_header->get_Res(0);
                        yMax = yMin + _header->get_yDim()*_header->get_Res(1);
                        zMax = zMin + _header->get_zDim()*_header->get_Res(2);

                        PS_LOG->addInfoMessage(LogInterface::step, "  : NEW Base position [" + QString::number(xMin) + ";" + QString::number(yMin) + ";" + QString::number(zMin) + "] " + "Resolution out : " + QString::number(_res));
                        PS_LOG->addInfoMessage(LogInterface::step, "  : SIZE (m) [" + QString::number(_xDim*_res) + ";" + QString::number(_yDim*_res) + ";" + QString::number(_zDim*_res) + "] " + "Resolution in : " + QString::number(_header->get_Res(0)));
                        PS_LOG->addInfoMessage(LogInterface::step, "  : NEW DIM [" + QString::number(_xDim) + ";" + QString::number(_yDim) + ";" + QString::number(_zDim) + "] " + "Resolution out : " + QString::number(_res));

                    }
                    delete reader;
                }

           }
#endif
      }

/*      WHY ? >> Cause LVOX Traversial Algorithm CRASH without */
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
        CT_Grid3D<int>*      hitGrid = CT_Grid3D<int>::createGrid3DFromXYZCoords(_hits_ModelName.completeName(), outResult, xMin, yMin, zMin, xMax, yMax, zMax, _res, -1, 0, true);
        CT_Grid3D<int>*      theoriticalGrid = new CT_Grid3D<int>(_theo_ModelName.completeName(), outResult, hitGrid->minX(), hitGrid->minY(), hitGrid->minZ(), hitGrid->xdim(), hitGrid->ydim(), hitGrid->zdim(), _res, -1, 0);
        CT_Grid3D<int>*      beforeGrid = new CT_Grid3D<int>(_bef_ModelName.completeName(), outResult, hitGrid->minX(), hitGrid->minY(), hitGrid->minZ(), hitGrid->xdim(), hitGrid->ydim(), hitGrid->zdim(), _res, -1, 0);
        CT_Grid3D<float>*   densityGrid = new CT_Grid3D<float>(_density_ModelName.completeName(), outResult, hitGrid->minX(), hitGrid->minY(), hitGrid->minZ(), hitGrid->xdim(), hitGrid->ydim(), hitGrid->zdim(), _res, -1, 0);

        hitGrid->addItemAttribute(new CT_StdItemAttributeT<bool>(_NiFlag_ModelName.completeName(), "LVOX_GRD_NI", outResult, true));
        theoriticalGrid->addItemAttribute(new CT_StdItemAttributeT<bool>(_NtFlag_ModelName.completeName(), "LVOX_GRD_NT", outResult, true));
        beforeGrid->addItemAttribute(new CT_StdItemAttributeT<bool>(_NbFlag_ModelName.completeName(), "LVOX_GRD_NB", outResult, true));
        densityGrid->addItemAttribute(new CT_StdItemAttributeT<bool>(_DensityFlag_ModelName.completeName(), "LVOX_GRD_DENSITY", outResult, true));


        CT_Grid3D<float>*   deltaInGrid = NULL;
        CT_Grid3D<float>*   deltaOutGrid = NULL;
        CT_Grid3D<float>*   deltaTheoritical = NULL;
        CT_Grid3D<float>*   deltaBefore = NULL;

        group->addItemDrawable(hitGrid);
        group->addItemDrawable(theoriticalGrid);
        group->addItemDrawable(beforeGrid);
        group->addItemDrawable(densityGrid);

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

        LVOX_ComputeDensityThread* densityThread = new LVOX_ComputeDensityThread(densityGrid, hitGrid, theoriticalGrid, beforeGrid, _effectiveRayThresh);
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

