/****************************************************************************

 Copyright (C) 2012-2012 Universit√© de Sherbrooke, Qu√©bec, CANADA
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
#include "ct_itemdrawable/model/outModel/ct_outstandardgroupmodel.h"
#include "ct_itemdrawable/model/inModel/ct_instandarditemdrawablemodel.h"
#include "ct_itemdrawable/model/outModel/ct_outstandarditemdrawablemodel.h"

#include "ct_result/model/inModel/ct_inresultmodelgroup.h"
#include "ct_result/model/outModel/ct_outresultmodelgroup.h"

#include "ct_step/abstract/ct_abstractsteploadfile.h"

#include "ct_result/ct_resultgroup.h"
#include "ct_itemdrawable/ct_scene.h"

#include "ct_view/ct_stepconfigurabledialog.h"          // Parameter window

#include "ct_itemdrawable/ct_scene.h"                   // This step takes a scene as input parameter

#include "algorithmewoo.h"                   // Using Woo algorithm for raytrace
#include "visitorraytracingincrement.h"      // Visitor of the raytracing algorithm

#include <QFileInfo>
#include <QDebug>                                       // Some printing output in debug mode


#define DEF_SearchInResult "r"
#define DEF_SearchInScene   "sc"

#define DEF_DeltaOutGridGroup "gg1"
#define DEF_DeltaInGridGroup "gg2"
#define DEF_DeltaBeforeGroup "gg3"
#define DEF_DeltaTheoriticalGroup "gg4"
#define DEF_CT_ResultRegularGridBeforeGroup "gg5"
#define DEF_CT_ResultRegularGridTheoriticalGroup "gg6"
#define DEF_CT_ResultRegularGridHitsGroup "gg7"
#define DEF_CT_ResultRegularGridDensityGroup "gg8"
#define DEF_CT_ResultScannerGroup "gg9"

#define DEF_DeltaOutGrid "ii1"
#define DEF_DeltaInGrid "ii2"
#define DEF_DeltaBefore "ii3"
#define DEF_DeltaTheoritical "ii4"
#define DEF_CT_ResultRegularGridBefore "ii5"
#define DEF_CT_ResultRegularGridTheoritical "ii6"
#define DEF_CT_ResultRegularGridHits "ii7"
#define DEF_CT_ResultRegularGridDensity "ii8"
#define DEF_CT_ResultScanner "ii9"

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
    _scanHFov = 40;
    _scanVFov = 40;
    _scanInitTheta = 0;
    _scanInitPhi = 90;
    _scanHRes = 0.2;
    _scanVRes = 0.2;
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

    CT_InStandardItemDrawableModel *item = new CT_InStandardItemDrawableModel(DEF_SearchInScene,
                                                                              CT_Scene::staticGetType(),
                                                                              tr("ScËne"));

    group->addItem(item);

    CT_InResultModelGroup *resultModel = new CT_InResultModelGroup(DEF_SearchInResult, group, tr("ScËne(s)"));

    addInResultModel(resultModel);
}

void StepComputeDensityAndDistances::createOutResultModelListProtected()
{
    CT_OutStandardGroupModel *group;
    CT_OutStandardItemDrawableModel *item;

    _nCategories = computeGridTools::getNCategories( _categoriesMarks );
    if ( _nCategories > 1 )
    {
        for ( int i = 0 ; i < _nCategories ; i++ )
        {

            //___________________________________________________________________
            group = new CT_OutStandardGroupModel(QString("g%1A").arg(i));
            item = new CT_OutStandardItemDrawableModel(QString("i%1A").arg(i),
                                                       new CT_RegularGridDouble(),
                                                       QString("CT_RegularGridDistOutCat%1").arg(i));
            group->addItem(item);
            addOutResultModel(new CT_OutResultModelGroup(QString("r%1A").arg(i), group,  QString("CT_RegularGridDistOutCat%1").arg(i)));

            //___________________________________________________________________
            group = new CT_OutStandardGroupModel(QString("g%1B").arg(i));
            item = new CT_OutStandardItemDrawableModel(QString("i%1B").arg(i),
                                                       new CT_RegularGridDouble(),
                                                       QString("CT_RegularGridDistInCat%1").arg(i));
            group->addItem(item);
            addOutResultModel(new CT_OutResultModelGroup(QString("r%1B").arg(i), group, QString("CT_RegularGridDistInCat%1").arg(i)));

            //___________________________________________________________________
            group = new CT_OutStandardGroupModel(QString("g%1C").arg(i));
            item = new CT_OutStandardItemDrawableModel(QString("i%1C").arg(i),
                                                       new CT_RegularGridInt(),
                                                       QString("CT_RegularGridHitsCat%1").arg(i));
            group->addItem(item);
            addOutResultModel(new CT_OutResultModelGroup(QString("r%1C").arg(i), group,  QString("CT_RegularGridHitsCat%1").arg(i)));
        }
    }

    // The results of this step are some single regular grids and a scanner

    //___________________________________________________________________
    group = new CT_OutStandardGroupModel(DEF_DeltaOutGridGroup);
    item = new CT_OutStandardItemDrawableModel(DEF_DeltaOutGrid,
                                               new CT_RegularGridInt(),
                                               "DeltaOutGrid");
    group->addItem(item);
    addOutResultModel(new CT_OutResultModelGroup("rr1", group,  "DeltaOutGrid"));

    //___________________________________________________________________
    group = new CT_OutStandardGroupModel(DEF_DeltaInGridGroup);
    item = new CT_OutStandardItemDrawableModel(DEF_DeltaInGrid,
                                               new CT_RegularGridInt(),
                                               "DeltaOutGrid");
    group->addItem(item);
    addOutResultModel(new CT_OutResultModelGroup("rr2", group,  "DeltaInGrid"));

    //___________________________________________________________________
    group = new CT_OutStandardGroupModel(DEF_DeltaBeforeGroup);
    item = new CT_OutStandardItemDrawableModel(DEF_DeltaBefore,
                                               new CT_RegularGridInt(),
                                               "DeltaBefore");
    group->addItem(item);
    addOutResultModel(new CT_OutResultModelGroup("rr3", group,  "DeltaBefore"));

    //___________________________________________________________________
    group = new CT_OutStandardGroupModel(DEF_DeltaTheoriticalGroup);
    item = new CT_OutStandardItemDrawableModel(DEF_DeltaTheoritical,
                                               new CT_RegularGridInt(),
                                               "DeltaTheoritical");
    group->addItem(item);
    addOutResultModel(new CT_OutResultModelGroup("rr4", group,  "DeltaTheoritical"));

    //___________________________________________________________________
    group = new CT_OutStandardGroupModel(DEF_CT_ResultRegularGridBeforeGroup);
    item = new CT_OutStandardItemDrawableModel(DEF_CT_ResultRegularGridBefore,
                                               new CT_RegularGridInt(),
                                               "CT_ResultRegularGridBefore");
    group->addItem(item);
    addOutResultModel(new CT_OutResultModelGroup("rr5", group,  "CT_ResultRegularGridBefore"));

    //___________________________________________________________________
    group = new CT_OutStandardGroupModel(DEF_CT_ResultRegularGridTheoriticalGroup);
    item = new CT_OutStandardItemDrawableModel(DEF_CT_ResultRegularGridTheoritical,
                                               new CT_RegularGridInt(),
                                               "CT_ResultRegularGridTheoritical");
    group->addItem(item);
    addOutResultModel(new CT_OutResultModelGroup("rr6", group,  "CT_ResultRegularGridTheoritical"));

    //___________________________________________________________________
    group = new CT_OutStandardGroupModel(DEF_CT_ResultRegularGridHitsGroup);
    item = new CT_OutStandardItemDrawableModel(DEF_CT_ResultRegularGridHits,
                                               new CT_RegularGridInt(),
                                               "CT_ResultRegularGridHits");
    group->addItem(item);
    addOutResultModel(new CT_OutResultModelGroup("rr7", group,  "CT_ResultRegularGridHits"));

    //___________________________________________________________________
    group = new CT_OutStandardGroupModel(DEF_CT_ResultRegularGridDensityGroup);
    item = new CT_OutStandardItemDrawableModel(DEF_CT_ResultRegularGridDensity,
                                               new CT_RegularGridDouble(),
                                               "CT_ResultRegularGridDensity");
    group->addItem(item);
    addOutResultModel(new CT_OutResultModelGroup("rr8", group,  "CT_ResultRegularGridDensity"));

    //___________________________________________________________________
    group = new CT_OutStandardGroupModel(DEF_CT_ResultScannerGroup);
    item = new CT_OutStandardItemDrawableModel(DEF_CT_ResultScanner,
                                               new CT_Scanner(),
                                               "CT_ResultScanner");
    group->addItem(item);
    addOutResultModel(new CT_OutResultModelGroup("rr9", group,  "CT_ResultScanner"));

}

void StepComputeDensityAndDistances::createConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->setStep(*this);

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

    // Get the future results
    CT_ResultGroup* outResultDeltaOut;
    CT_ResultGroup* outResultDeltaIn;
    CT_ResultGroup* outResultDeltaBefore;
    CT_ResultGroup* outResultDeltaTheoritical;
    CT_ResultGroup* outResultBefore;
    CT_ResultGroup* outResultTheoritical;
    CT_ResultGroup* outResultHit;
    CT_ResultGroup* outResultDensity;
    CT_ResultGroup* outResultScanner;

    QList<CT_ResultGroup*> outResultList = getOutResultList();

    if ( _nCategories > 1 )
    {
        outResultDeltaOut = outResultList.at(3*_nCategories);
        outResultDeltaIn = outResultList.at(3*_nCategories+1);
        outResultDeltaBefore = outResultList.at(3*_nCategories+2);
        outResultDeltaTheoritical = outResultList.at(3*_nCategories+3);
        outResultBefore = outResultList.at(3*_nCategories+4);
        outResultTheoritical = outResultList.at(3*_nCategories+5);
        outResultHit = outResultList.at(3*_nCategories+6);
        outResultDensity = outResultList.at(3*_nCategories+7);
        outResultScanner = outResultList.at(3*_nCategories+8);
    }

    else
    {
        outResultDeltaOut = outResultList.at(0);
        outResultDeltaIn = outResultList.at(1);
        outResultDeltaBefore = outResultList.at(2);
        outResultDeltaTheoritical = outResultList.at(3);
        outResultBefore = outResultList.at(4);
        outResultTheoritical = outResultList.at(5);
        outResultHit = outResultList.at(6);
        outResultDensity = outResultList.at(7);
        outResultScanner = outResultList.at(8);
    }

    // Creating a new scanner from the input parameters
    CT_Scanner* scanner = new CT_Scanner(NULL,
                                         0,
                                         outResultScanner,
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
    CT_RegularGridDouble*   deltaOutGrid = NULL;
    CT_RegularGridDouble*   deltaInGrid = NULL;
    CT_RegularGridDouble*   deltaBefore = NULL;
    CT_RegularGridDouble*   deltaTheoritical = NULL;
    CT_RegularGridInt*      beforeGrid = NULL;
    CT_RegularGridInt*      theoriticalGrid = NULL;
    CT_RegularGridInt*      hitGrid = NULL;
    CT_RegularGridDouble*   densityGrid = NULL;

    QList< CT_RegularGridInt* > categoryHitsGridList;
    QList< CT_RegularGridDouble* > categoryDeltaInGridList;
    QList< CT_RegularGridDouble* > categoryDeltaOutGridList;

    if ( _nCategories > 1 )
    {
        // Filling the lists
        for ( int i = 0 ; i < _nCategories ; i++ )
        {
            categoryHitsGridList.push_back(NULL);
            categoryDeltaInGridList.push_back(NULL);
            categoryDeltaOutGridList.push_back(NULL);
        }
    }

    // Computing the different grids
    computeGridTools::computeTheoriticalGridAndDistances( ctMin, ctMax, _res, *scanner, 0, outResultTheoritical, theoriticalGrid, 1, outResultDeltaTheoritical, deltaTheoritical );
    computeGridTools::computeBeforeGridAndDistances( ctMin, ctMax, _res, scene->getPointCloud(), *scanner, 2, outResultBefore, beforeGrid, 3, outResultDeltaBefore, deltaBefore );

    if ( _nCategories > 1 )
        computeGridTools::computeHitGridAndDistancesAndCategories( ctMin, ctMax, _res, scene->getPointCloud(), *scanner, _intensityThresh, _greaterThanThresh, 4, outResultHit, hitGrid, 5, outResultDeltaIn, deltaInGrid, 6, outResultDeltaOut, deltaOutGrid, categoryHitsGridList, categoryDeltaInGridList, categoryDeltaOutGridList, _nCategories, _categoriesMarks );
    else
        computeGridTools::computeHitGridAndDistances( ctMin, ctMax, _res, scene->getPointCloud(), *scanner, _intensityThresh, _greaterThanThresh, 4, outResultHit, hitGrid, 5, outResultDeltaIn, deltaInGrid, 6, outResultDeltaOut, deltaOutGrid );

    densityGrid = computeGridTools::computeDensityGrid(7, outResultDensity, hitGrid, theoriticalGrid, beforeGrid, _effectiveRayThresh );

    if ( _nCategories > 1 )
    {
        // Setting the category grids result
        for ( int i = 0 ; i < _nCategories ; i++ )
        {
            categoryDeltaOutGridList[i]->changeResult(outResultList.at((3*i))); // 3 = deltaOut, deltaIn Hit
            categoryDeltaInGridList[i]->changeResult(outResultList.at((3*i)+1));
            categoryHitsGridList[i]->changeResult(outResultList.at((3*i)+2));
        }
    }

    // Setting the output grids
    CT_StandardItemGroup *group;
    CT_OutAbstractStandardGroupModel *model;
    CT_OutAbstractItemDrawableModel *itemModel;

    model = (CT_OutAbstractStandardGroupModel*)getOutModelForCreation(outResultScanner, DEF_CT_ResultScannerGroup);
    group = new CT_StandardItemGroup(model, 0, outResultScanner);
    outResultScanner->addGroup(group);
    itemModel = (CT_OutAbstractItemDrawableModel*)getOutModelForCreation(outResultScanner, DEF_CT_ResultScanner);
    scanner->setModel(itemModel);
    group->addItemDrawable(scanner);

    model = (CT_OutAbstractStandardGroupModel*)getOutModelForCreation(outResultDensity, DEF_CT_ResultRegularGridDensityGroup);
    group = new CT_StandardItemGroup(model, 0, outResultDensity);
    outResultDensity->addGroup(group);
    itemModel = (CT_OutAbstractItemDrawableModel*)getOutModelForCreation(outResultDensity, DEF_CT_ResultRegularGridDensity);
    densityGrid->setModel(itemModel);
    group->addItemDrawable(densityGrid);

    model = (CT_OutAbstractStandardGroupModel*)getOutModelForCreation(outResultHit, DEF_CT_ResultRegularGridHitsGroup);
    group = new CT_StandardItemGroup(model, 0, outResultHit);
    outResultHit->addGroup(group);
    itemModel = (CT_OutAbstractItemDrawableModel*)getOutModelForCreation(outResultHit, DEF_CT_ResultRegularGridHits);
    hitGrid->setModel(itemModel);
    group->addItemDrawable(hitGrid);

    model = (CT_OutAbstractStandardGroupModel*)getOutModelForCreation(outResultTheoritical, DEF_CT_ResultRegularGridTheoriticalGroup);
    group = new CT_StandardItemGroup(model, 0, outResultTheoritical);
    outResultTheoritical->addGroup(group);
    itemModel = (CT_OutAbstractItemDrawableModel*)getOutModelForCreation(outResultTheoritical, DEF_CT_ResultRegularGridTheoritical);
    theoriticalGrid->setModel(itemModel);
    group->addItemDrawable(theoriticalGrid);

    model = (CT_OutAbstractStandardGroupModel*)getOutModelForCreation(outResultBefore, DEF_CT_ResultRegularGridBeforeGroup);
    group = new CT_StandardItemGroup(model, 0, outResultBefore);
    outResultBefore->addGroup(group);
    itemModel = (CT_OutAbstractItemDrawableModel*)getOutModelForCreation(outResultBefore, DEF_CT_ResultRegularGridBefore);
    beforeGrid->setModel(itemModel);
    group->addItemDrawable(beforeGrid);

    model = (CT_OutAbstractStandardGroupModel*)getOutModelForCreation(outResultDeltaTheoritical, DEF_DeltaTheoriticalGroup);
    group = new CT_StandardItemGroup(model, 0, outResultDeltaTheoritical);
    outResultDeltaTheoritical->addGroup(group);
    itemModel = (CT_OutAbstractItemDrawableModel*)getOutModelForCreation(outResultDeltaTheoritical, DEF_DeltaTheoritical);
    deltaTheoritical->setModel(itemModel);
    group->addItemDrawable(deltaTheoritical);

    model = (CT_OutAbstractStandardGroupModel*)getOutModelForCreation(outResultDeltaBefore, DEF_DeltaBeforeGroup);
    group = new CT_StandardItemGroup(model, 0, outResultDeltaBefore);
    outResultDeltaBefore->addGroup(group);
    itemModel = (CT_OutAbstractItemDrawableModel*)getOutModelForCreation(outResultDeltaBefore, DEF_DeltaBefore);
    deltaBefore->setModel(itemModel);
    group->addItemDrawable(deltaBefore);

    model = (CT_OutAbstractStandardGroupModel*)getOutModelForCreation(outResultDeltaIn, DEF_DeltaInGridGroup);
    group = new CT_StandardItemGroup(model, 0, outResultDeltaIn);
    outResultDeltaIn->addGroup(group);
    itemModel = (CT_OutAbstractItemDrawableModel*)getOutModelForCreation(outResultDeltaIn, DEF_DeltaInGrid);
    deltaInGrid->setModel(itemModel);
    group->addItemDrawable(deltaInGrid);

    model = (CT_OutAbstractStandardGroupModel*)getOutModelForCreation(outResultDeltaOut, DEF_DeltaOutGridGroup);
    group = new CT_StandardItemGroup(model, 0, outResultDeltaOut);
    outResultDeltaOut->addGroup(group);
    itemModel = (CT_OutAbstractItemDrawableModel*)getOutModelForCreation(outResultDeltaOut, DEF_DeltaOutGrid);
    deltaOutGrid->setModel(itemModel);
    group->addItemDrawable(deltaOutGrid);


    if ( _nCategories > 1 )
    {
        for ( int i = 0 ; i < _nCategories ; i++ )
        {
            model = (CT_OutAbstractStandardGroupModel*)getOutModelForCreation(outResultList.at(3*i), QString("g%1A").arg(i));
            group = new CT_StandardItemGroup(model, 0, outResultList.at(3*i));
            outResultList.at(3*i)->addGroup(group);
            itemModel = (CT_OutAbstractItemDrawableModel*)getOutModelForCreation(outResultList.at(3*i), QString("i%1A").arg(i));
            categoryDeltaOutGridList[i]->setModel(itemModel);
            group->addItemDrawable(categoryDeltaOutGridList[i]);

            model = (CT_OutAbstractStandardGroupModel*)getOutModelForCreation(outResultList.at((3*i)+1), QString("g%1B").arg(i));
            group = new CT_StandardItemGroup(model, 0, outResultList.at((3*i)+1));
            outResultList.at((3*i)+1)->addGroup(group);
            itemModel = (CT_OutAbstractItemDrawableModel*)getOutModelForCreation(outResultList.at((3*i)+1), QString("i%1B").arg(i));
            categoryDeltaInGridList[i]->setModel(itemModel);
            group->addItemDrawable(categoryDeltaInGridList[i]);

            model = (CT_OutAbstractStandardGroupModel*)getOutModelForCreation(outResultList.at((3*i)+2), QString("g%1C").arg(i));
            group = new CT_StandardItemGroup(model, 0, outResultList.at((3*i)+2));
            outResultList.at((3*i)+2)->addGroup(group);
            itemModel = (CT_OutAbstractItemDrawableModel*)getOutModelForCreation(outResultList.at((3*i)+1), QString("i%1C").arg(i));
            categoryHitsGridList[i]->setModel(itemModel);
            group->addItemDrawable(categoryHitsGridList[i]);
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
