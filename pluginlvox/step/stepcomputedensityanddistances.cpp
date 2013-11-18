/****************************************************************************

 Copyright (C) 2012-2012 UniversitÃ© de Sherbrooke, QuÃ©bec, CANADA
                     All rights reserved.

 Contact :  richard.fournier@usherbrooke.ca
            jean-francois.cote@nrcan-rncan.gc.ca
            joris.ravaglia@gmail.com

 Developers : Joris RAVAGLIA

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

#include "stepcomputedensityanddistances.h"

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
#include "ct_itemdrawable/ct_regulargriddouble.h"
#include "ct_itemdrawable/ct_regulargridint.h"
#include "ct_itemdrawable/ct_scanner.h"

#include "ct_step/abstract/ct_abstractsteploadfile.h"
#include "ct_view/ct_stepconfigurabledialog.h"          // Parameter window

#include "ct_itemdrawable/ct_scene.h"                   // This step takes a scene as input parameter

#include "algorithmewoo.h"                              // Using Woo algorithm for raytrace
#include "visitorraytracingincrement.h"                 // Visitor of the raytracing algorithm

#include <QFileInfo>
#include <QDebug>                                       // Some printing output in debug mode


#define DEF_SearchInResult "r"
#define DEF_SearchInScene   "sc"

// Alias for indexing out models
#define DEF_resultOut_grid "grid"
#define DEF_groupOut_base "base"
#define DEF_itemOut_h "h"
#define DEF_itemOut_dout "dout"
#define DEF_itemOut_din "din"
#define DEF_itemOut_deltaout "deltaout"
#define DEF_itemOut_deltain "deltain"
#define DEF_itemOut_deltabef "deltabef"
#define DEF_itemOut_deltatheo "deltatheo"
#define DEF_itemOut_bef "bef"
#define DEF_itemOut_theo "theo"
#define DEF_itemOut_hits "hits"
#define DEF_itemOut_density "density"
#define DEF_itemOut_scan "scan"

StepComputeDensityAndDistances::StepComputeDensityAndDistances(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    //********************************************//
    //              Attributes of LVox            //
    //********************************************//
    _res = 0.5;
    _intensityThresh = 10;
    _greaterThanThresh = true;

    _effectiveRayThresh = 10;

    _nCategories = 1;
    _categoriesMarks.resize( 4, -1 );
    _saveGrid = true;

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

QString StepComputeDensityAndDistances::getStepDescription() const
{
    // Gives the descrption to print in the GUI
    return tr("Discretise l'espace en voxels et calcule un indice de denste par voxel et les distances associees");
}

CT_VirtualAbstractStep* StepComputeDensityAndDistances::createNewInstance(CT_StepInitializeData &dataInit)
{
    // Creates an instance of this step
    return new StepComputeDensityAndDistances(dataInit);
}

void StepComputeDensityAndDistances::createInResultModelListProtected()
{
    // results with Scenes
    CT_InOneOrMoreGroupModel *group = new CT_InOneOrMoreGroupModel();

    CT_InStandardItemDrawableModel *item = new CT_InStandardItemDrawableModel(DEF_SearchInScene, CT_Scene::staticGetType(), tr("Scène"));
    group->addItem(item);

    CT_InResultModelGroup *resultModel = new CT_InResultModelGroup(DEF_SearchInResult, group, tr("Scène(s)"));
    addInResultModel(resultModel);
}

void StepComputeDensityAndDistances::createOutResultModelListProtected()
{
    _nCategories = computeGridTools::getNCategories( _categoriesMarks );

    CT_OutStandardGroupModel *groupOutModel_base = new CT_OutStandardGroupModel(DEF_groupOut_base, new CT_StandardItemGroup(), tr("base"));

    CT_OutStandardItemDrawableModel *itemOutModel_h;
    CT_OutStandardItemDrawableModel *itemOutModel_dout;
    CT_OutStandardItemDrawableModel *itemOutModel_din;

    if (_nCategories > 1)
    {
        for (unsigned int i = 0 ; i < _nCategories ; i++ )
        {
            itemOutModel_h = new CT_OutStandardItemDrawableModel(QString(DEF_itemOut_h).append(QString::number(i)), new CT_RegularGridInt(), tr("HitsCat").append(QString::number(i)));
            itemOutModel_dout = new CT_OutStandardItemDrawableModel(QString(DEF_itemOut_dout).append(QString::number(i)), new CT_RegularGridDouble(), tr("DistOutCat").append(QString::number(i)));
            itemOutModel_din = new CT_OutStandardItemDrawableModel(QString(DEF_itemOut_din).append(QString::number(i)), new CT_RegularGridDouble(), tr("DistInCat").append(QString::number(i)));

            groupOutModel_base->addItem(itemOutModel_h);
            groupOutModel_base->addItem(itemOutModel_dout);
            groupOutModel_base->addItem(itemOutModel_din);
        }
    }


    CT_OutStandardItemDrawableModel *itemOutModel_deltaout = new CT_OutStandardItemDrawableModel(DEF_itemOut_deltaout, new CT_RegularGridDouble(), tr("DeltaOut"));
    CT_OutStandardItemDrawableModel *itemOutModel_deltain = new CT_OutStandardItemDrawableModel(DEF_itemOut_deltain, new CT_RegularGridDouble(), tr("DeltaIn"));
    CT_OutStandardItemDrawableModel *itemOutModel_deltabef = new CT_OutStandardItemDrawableModel(DEF_itemOut_deltabef, new CT_RegularGridDouble(), tr("DeltaBefore"));
    CT_OutStandardItemDrawableModel *itemOutModel_deltatheo = new CT_OutStandardItemDrawableModel(DEF_itemOut_deltatheo, new CT_RegularGridDouble(), tr("DeltaTheorical"));
    CT_OutStandardItemDrawableModel *itemOutModel_bef = new CT_OutStandardItemDrawableModel(DEF_itemOut_bef, new CT_RegularGridInt(), tr("Before"));
    CT_OutStandardItemDrawableModel *itemOutModel_theo = new CT_OutStandardItemDrawableModel(DEF_itemOut_theo, new CT_RegularGridInt(), tr("Theorical"));
    CT_OutStandardItemDrawableModel *itemOutModel_hits = new CT_OutStandardItemDrawableModel(DEF_itemOut_hits, new CT_RegularGridInt(), tr("Hits"));
    CT_OutStandardItemDrawableModel *itemOutModel_density = new CT_OutStandardItemDrawableModel(DEF_itemOut_density, new CT_RegularGridDouble(), tr("Density"));
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

void StepComputeDensityAndDistances::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    //********************************************//
    //              Attributes of LVox            //
    //********************************************//
    configDialog->addDouble(tr("Size of a voxel"),tr("meters"),0.0001,10000,2, _res );
    configDialog->addDouble(tr("Minimum intensity taken in account"),tr(""),-100000,100000,2, _intensityThresh );
    configDialog->addBool("Consider the threshold as a minimum value","","",_greaterThanThresh);
    configDialog->addDouble(tr("Minimum number of effective ray in a voxel to take it into account"),tr(""),-100000,100000,2, _effectiveRayThresh );

    configDialog->addInt(tr("Benchmark between first and second category"), tr(""), -10000, 1000000, _categoriesMarks[0]);
    configDialog->addInt(tr("Benchmark between second and thirs category"), tr(""), -10000, 1000000, _categoriesMarks[1]);
    configDialog->addInt(tr("Benchmark between third and fourth category"), tr(""), -10000, 1000000, _categoriesMarks[2]);
    configDialog->addInt(tr("Benchmark between fourth and fifth category"), tr(""), -10000, 1000000, _categoriesMarks[3]);

    configDialog->addBool("Save in LVox file in \"LVox Density File\" format (.ldf)","","",_saveGrid);

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

void StepComputeDensityAndDistances::compute()
{
    // Get the input file name and path
    //QString filePathAndName = (dynamic_cast<CT_VirtualAbstractStep*> (this->parentStep()))->getAllSettings()->at(0).toString();

    Step* parent = this;
    while (parent->parentStep() != NULL) {parent = parent->parentStep();}

    CT_AbstractStepLoadFile* loadFileStep = dynamic_cast<CT_AbstractStepLoadFile*>(parent);
    if (loadFileStep == NULL) {
        return;
    }
    QString filePathAndName = loadFileStep->getFilePath();


    QFileInfo fileInfo ( filePathAndName );
    _inputInFilePath = fileInfo.absolutePath()+QString("/");
    _inputInFileName = fileInfo.baseName();

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
                                         _scanClockWise );

    // Convert the bbox of the scene into a CT_Point
    CT_Point ctMin, ctMax;
    ctMin.x = scene->xMin(); ctMin.y = scene->yMin(); ctMin.z = scene->zMin();
    ctMax.x = scene->xMax(); ctMax.y = scene->yMax(); ctMax.z = scene->zMax();

    // Declaring the output grids
    CT_RegularGridDouble*   deltaOutGrid = new CT_RegularGridDouble(itemOutModel_deltaout, outResult, ctMax, ctMin, _res, 0);
    CT_RegularGridDouble*   deltaInGrid = new CT_RegularGridDouble(itemOutModel_deltain, outResult, ctMax, ctMin, _res, 0);
    CT_RegularGridDouble*   deltaBefore = new CT_RegularGridDouble(itemOutModel_deltabef, outResult, ctMax, ctMin, _res, 0);
    CT_RegularGridDouble*   deltaTheoritical = new CT_RegularGridDouble(itemOutModel_deltatheo, outResult, ctMax, ctMin, _res, 0);
    CT_RegularGridInt*      beforeGrid = new CT_RegularGridInt(itemOutModel_bef, outResult, ctMax, ctMin, _res, 0);;
    CT_RegularGridInt*      theoriticalGrid = new CT_RegularGridInt(itemOutModel_theo, outResult, ctMax, ctMin, _res, 0);;
    CT_RegularGridInt*      hitGrid = new CT_RegularGridInt(itemOutModel_hits, outResult, ctMax, ctMin, _res, 0);
    CT_RegularGridDouble*   densityGrid = new CT_RegularGridDouble(itemOutModel_density, outResult, ctMax, ctMin, _res, 0);

    QList< CT_RegularGridInt* > categoryHitsGridList;
    QList< CT_RegularGridDouble* > categoryDeltaInGridList;
    QList< CT_RegularGridDouble* > categoryDeltaOutGridList;

    if ( _nCategories > 1 )
    {
        // Filling the lists
        for (unsigned int i = 0 ; i < _nCategories ; i++ )
        {
            CT_OutStandardItemDrawableModel* itemOutModel_h = (CT_OutStandardItemDrawableModel*)getOutModelForCreation(outResult, QString(DEF_itemOut_h).append(QString::number(i)));
            CT_OutStandardItemDrawableModel* itemOutModel_dout = (CT_OutStandardItemDrawableModel*)getOutModelForCreation(outResult, QString(DEF_itemOut_dout).append(QString::number(i)));
            CT_OutStandardItemDrawableModel* itemOutModel_din = (CT_OutStandardItemDrawableModel*)getOutModelForCreation(outResult, QString(DEF_itemOut_din).append(QString::number(i)));


            categoryHitsGridList.push_back(new CT_RegularGridInt(itemOutModel_h, outResult, ctMax, ctMin, _res, 0));
            categoryDeltaOutGridList.push_back(new CT_RegularGridDouble(itemOutModel_dout, outResult, ctMax, ctMin, _res, 0));
            categoryDeltaInGridList.push_back(new CT_RegularGridDouble(itemOutModel_din, outResult, ctMax, ctMin, _res, 0));
        }
    }

    // Computing the different grids
    computeGridTools::computeTheoriticalGridAndDistances( ctMin, ctMax, _res, scanner, theoriticalGrid, deltaTheoritical );
    computeGridTools::computeBeforeGridAndDistances( ctMin, ctMax, _res, scene->getPointCloud(), scanner, beforeGrid, deltaBefore );

    if ( _nCategories > 1 )
        computeGridTools::computeHitGridAndDistancesAndCategories( ctMin, ctMax, _res, scene->getPointCloud(), scanner, _intensityThresh, _greaterThanThresh, hitGrid, deltaInGrid, deltaOutGrid, categoryHitsGridList, categoryDeltaInGridList, categoryDeltaOutGridList, _nCategories, _categoriesMarks );
    else
        computeGridTools::computeHitGridAndDistances( ctMin, ctMax, _res, scene->getPointCloud(), scanner, _intensityThresh, _greaterThanThresh, hitGrid, deltaInGrid, deltaOutGrid );

    computeGridTools::computeDensityGrid(densityGrid, hitGrid, theoriticalGrid, beforeGrid, _effectiveRayThresh );


    baseGroup->addItemDrawable(deltaOutGrid);
    baseGroup->addItemDrawable(deltaInGrid);
    baseGroup->addItemDrawable(deltaBefore);
    baseGroup->addItemDrawable(deltaTheoritical);
    baseGroup->addItemDrawable(beforeGrid);
    baseGroup->addItemDrawable(theoriticalGrid);
    baseGroup->addItemDrawable(hitGrid);
    baseGroup->addItemDrawable(densityGrid);

    if ( _nCategories > 1 )
    {
        for (unsigned int i = 0 ; i < _nCategories ; i++ )
        {
            baseGroup->addItemDrawable(categoryHitsGridList[i]);
            baseGroup->addItemDrawable(categoryDeltaOutGridList[i]);
            baseGroup->addItemDrawable(categoryDeltaInGridList[i]);
        }
    }

    // Save results in a file if demanded
    if ( _saveGrid )
    {
        computeGridTools::saveDensityAndDistancesResult( densityGrid,
                                                         hitGrid,
                                                         theoriticalGrid,
                                                         beforeGrid,
                                                         deltaTheoritical,
                                                         deltaBefore,
                                                         deltaInGrid,
                                                         deltaOutGrid,
                                                         categoryHitsGridList,
                                                         categoryDeltaInGridList,
                                                         categoryDeltaOutGridList,
                                                         _nCategories,
                                                         _categoriesMarks,
                                                         _inputInFilePath,
                                                         _inputInFileName );
    }
}
