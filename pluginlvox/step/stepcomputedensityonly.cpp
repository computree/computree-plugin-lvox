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

#include "stepcomputedensityonly.h"

#include "ct_itemdrawable/model/inModel/ct_inoneormoregroupmodel.h"
#include "ct_itemdrawable/model/outModel/ct_outstandardgroupmodel.h"
#include "ct_itemdrawable/model/inModel/ct_instandarditemdrawablemodel.h"
#include "ct_itemdrawable/model/outModel/ct_outstandarditemdrawablemodel.h"

#include "ct_result/model/inModel/ct_inresultmodelgroup.h"
#include "ct_result/model/outModel/ct_outresultmodelgroup.h"

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

StepComputeDensityOnly::StepComputeDensityOnly(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
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

QString StepComputeDensityOnly::getStepDescription() const
{
    // Gives the descrption to print in the GUI
    return tr("Discretise l'espace en voxels et calcule un indice de denste par voxel");
}

CT_VirtualAbstractStep* StepComputeDensityOnly::createNewInstance(CT_StepInitializeData &dataInit)
{
    // Creates an instance of this step
    return new StepComputeDensityOnly(dataInit);
}

void StepComputeDensityOnly::createInResultModelListProtected()
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

void StepComputeDensityOnly::createOutResultModelListProtected()
{
    // Get the number of categories
    _nCategories = computeGridTools::getNCategories( _categoriesMarks );

    if ( _nCategories > 1 )
    {
        for ( unsigned int i = 0 ; i < _nCategories ; i++ )
        {
            addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultGroup(), QList<CT_AbstractItemDrawable*>() << new CT_RegularGridInt(), QString("CT_ResultRegularGridHitsCategoryFromScanner").append(QString().number(i)) ) );
        }
    }

    // The results of this step are some single regular grids and a scanner
    addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultGroup(), QList<CT_AbstractItemDrawable*>() << new CT_RegularGridInt(), "CT_ResultRegularGridTheoritical"));
    addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultGroup(), QList<CT_AbstractItemDrawable*>() << new CT_RegularGridInt(), "CT_ResultRegularGridBefore"));
    addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultGroup(), QList<CT_AbstractItemDrawable*>() << new CT_RegularGridInt(), "CT_ResultRegularGridHits"));

    addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultGroup(), QList<CT_AbstractItemDrawable*>() << new CT_RegularGridDouble(), "CT_ResultRegularGridDensity"));

    addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultGroup(), QList<CT_AbstractItemDrawable*>() << new CT_Scanner(), "CT_ResultScanner"));
}

void StepComputeDensityOnly::createConfigurationDialog()
{
    _configDialog = new CT_StepConfigurableDialog();
    _configDialog->setStep(*this);

//********************************************//
//              Attributes of LVox            //
//********************************************//
    _configDialog->addDouble(tr("Size of a voxel"),tr("meters"),0.0001,10000,2, _res );
    _configDialog->addDouble(tr("Minimum intensity taken in account"),tr(""),-100000,100000,2, _intensityThresh );
    _configDialog->addBool("Consider the threshold as a minimum value","","",_greaterThanThresh);
    _configDialog->addDouble(tr("Minimum number of effective ray in a voxel to take it into account"),tr(""),-100000,100000,2, _effectiveRayThresh );

    _configDialog->addInt(tr("Benchmark between first and second category"), tr(""), -10000, 1000000, _categoriesMarks[0]);
    _configDialog->addInt(tr("Benchmark between second and thirs category"), tr(""), -10000, 1000000, _categoriesMarks[1]);
    _configDialog->addInt(tr("Benchmark between third and fourth category"), tr(""), -10000, 1000000, _categoriesMarks[2]);
    _configDialog->addInt(tr("Benchmark between fourth and fifth category"), tr(""), -10000, 1000000, _categoriesMarks[3]);

    _configDialog->addBool("Save in LVox file in \"LVox Density File\" format (.ldf)","","",_saveGrid);

//********************************************//
//           Attributes of the scanner        //
//********************************************//
    _configDialog->addDouble(tr("Position du scanner x"),tr("metres"),-10000,10000,2, _scanPosX );
    _configDialog->addDouble(tr("Position du scanner y"),tr("metres"),-10000,10000,2, _scanPosY );
    _configDialog->addDouble(tr("Position du scanner z"),tr("metres"),-10000,10000,2, _scanPosZ );

    _configDialog->addDouble(tr("Champ de vue horizontal du scanner"),tr("degres"),0.0001,360,2, _scanHFov );
    _configDialog->addDouble(tr("Champ de vue vertical du scanner"),tr("degres"),0.0001,360,2, _scanVFov );

    _configDialog->addDouble(tr("Theta initial du scanner"),tr("degres"),-359.99,359.99,2, _scanInitTheta );
    _configDialog->addDouble(tr("Phi initial du scanner"),tr("degres"),0,180,2, _scanInitPhi );

    _configDialog->addDouble(tr("Resolution angulaire horizontale du scanner"),tr("degres"),0.0001,10000,3, _scanHRes );
    _configDialog->addDouble(tr("Resolution angulaire verticale du scanner"),tr("degres"),0.0001,10000,3, _scanVRes );

    _configDialog->addBool("Scanner sens horaire","","",_scanClockWise);
}

void StepComputeDensityOnly::compute()
{
    // Get the input file name and path
    QString filePathAndName = (dynamic_cast<CT_VirtualAbstractStep*> (this->parentStep()))->getAllSettings().at(0).toString();
    QFileInfo fileInfo ( filePathAndName );
    _inputInFilePath = fileInfo.absolutePath()+QString("/");
    _inputInFileName = fileInfo.baseName();

    // Get the input scene parameter
    CT_ResultGroup *inResult = getInputResults().first();
    CT_InAbstractItemDrawableModel *inSceneModel = (CT_InAbstractItemDrawableModel*) getInModelForResearch(inResult, DEF_SearchInScene);
    inResult->recursiveBeginIterateItems(*inSceneModel);
    const CT_Scene* scene = (CT_Scene*) inResult->recursiveNextItem()->findFirstItem(inSceneModel);
    assert (scene!=NULL);

    // Get the future results
    CT_ResultGroup* outResultBefore;
    CT_ResultGroup* outResultTheoritical;
    CT_ResultGroup* outResultHit;
    CT_ResultGroup* outResultDensity;
    CT_ResultGroup* outResultScanner;

    if ( _nCategories > 1 )
    {
        outResultBefore = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(_nCategories));
        outResultTheoritical = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(_nCategories+1));
        outResultHit = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(_nCategories+2));
        outResultDensity = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(_nCategories+3));
        outResultScanner = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(_nCategories+4));
    }

    else
    {
        outResultBefore = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(0));
        outResultTheoritical = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(1));
        outResultHit = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(2));
        outResultDensity = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(3));
        outResultScanner = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(4));
    }

    // Creating a new scanner from the input parameters
    CT_Scanner* scanner = new CT_Scanner(0,
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
    CT_RegularGridDouble*   densityGrid = NULL;
    CT_RegularGridInt*      hitGrid = NULL;
    CT_RegularGridInt*      theoriticalGrid = NULL;
    CT_RegularGridInt*      beforeGrid = NULL;

    // Computing the different grids
    hitGrid = computeGridTools::computeHitGrid(0, outResultHit, ctMin, ctMax, _res, scene->getPointCloud(), _intensityThresh, _greaterThanThresh );
    theoriticalGrid = computeGridTools::computeTheoriticalGrid( 1, outResultTheoritical, ctMin, ctMax, _res, *scanner );
    beforeGrid = computeGridTools::computeBeforeGrid( 2, outResultBefore, ctMin, ctMax, _res, scene->getPointCloud(), *scanner );
    densityGrid = computeGridTools::computeDensityGrid( 3, outResultDensity, hitGrid, theoriticalGrid, beforeGrid, _effectiveRayThresh );

    // Declaring a list of grids in case categories are demanded
    QList< CT_RegularGridInt* > categoryHitGridList;
    QList< qint64 >             idList;
    QList< Result* >            resultList;

    // Computing the category hit grids is done only if there is several categories
    if ( _nCategories > 1 )
    {
        // Filling the list with as many NULL pointer as the number of categories and set the appropriate id and result
        for ( unsigned int i = 0 ; i < _nCategories ; i++ )
        {
            categoryHitGridList.push_back( NULL );
            idList.push_back(4+i);
            resultList.push_back( dynamic_cast<CT_ResultGroup*>(getOutResultList().at(i)) );
        }

        computeGridTools::computeCategoriesHitGrids(ctMin, ctMax, _res, scene->getPointCloud(), _intensityThresh, _greaterThanThresh, categoryHitGridList, idList, resultList, _nCategories, _categoriesMarks );
    }

    // Attaching the resulting grids to their effective result
    outResultDensity->setItemDrawable( densityGrid );
    outResultHit->setItemDrawable( hitGrid );
    outResultTheoritical->setItemDrawable( theoriticalGrid );
    outResultBefore->setItemDrawable( beforeGrid );

    if ( _nCategories > 1 )
    {
        for ( unsigned int i = 0 ; i < _nCategories ; i++ )
        {
            dynamic_cast<CT_ResultGroup*>(getOutResultList().at(i))->setItemDrawable( categoryHitGridList[i] );
        }
    }

    // Attaching the scanner to the corresponding result
    outResultScanner->setItemDrawable( scanner );

    // Writes the output result in a separated file if demanded
    if ( _saveGrid )
    {
        computeGridTools::saveDensityOnlyResult( densityGrid, hitGrid, theoriticalGrid, beforeGrid, categoryHitGridList, _nCategories, _categoriesMarks, _inputInFilePath, _inputInFileName+QString(".ldf") );
    }

    theoriticalGrid->saveAsText("ResultatAvecRaytracing.txt");
}
