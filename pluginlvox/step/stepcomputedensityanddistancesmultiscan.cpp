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

#include "stepcomputedensityanddistancesmultiscan.h"

#include "ct_itemdrawable/model/inModel/ct_inoneormoregroupmodel.h"
#include "ct_itemdrawable/model/outModel/ct_outstandardgroupmodel.h"
#include "ct_itemdrawable/model/inModel/ct_instandarditemdrawablemodel.h"
#include "ct_itemdrawable/model/outModel/ct_outstandarditemdrawablemodel.h"

#include "ct_result/model/inModel/ct_inresultmodelgroup.h"
#include "ct_result/model/outModel/ct_outresultmodelgroup.h"

#include "ct_result/ct_resultgroup.h"
#include "ct_itemdrawable/ct_scene.h"

#include "ct_view/ct_stepconfigurabledialog.h"                          // Parameter window

#include "ct_itemdrawable/ct_scene.h"                                   // This step takes a scene as input parameter

#include "algorithmewoo.h"                                   // Using Woo algorithm for raytrace
#include "visitorraytracingincrement.h"                      // Visitor of the raytracing algorithm

#include <QFileInfo>
#include <QDebug>                                                       // Some printing output in debug mode

#define DEF_SearchInResult "r"
#define DEF_SearchInScene   "sc"
#define DEF_SearchInResultScanner "rs"
#define DEF_SearchInScanner   "sca"

StepComputeDensityAndDistancesMultiScan::StepComputeDensityAndDistancesMultiScan(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _res = 0.5;
    _intensityThresh = 10;
    _greaterThanThresh = true;

    _effectiveRayThresh = 10;

    _nCategories = 1;
    _categoriesMarks.resize( 4, -1 );
    _saveGrid = true;
}

QString StepComputeDensityAndDistancesMultiScan::getStepDescription() const
{
    // Gives the descrption to print in the GUI
    return tr("Discretise l'espace en voxels et calcule un indice de denste par voxel et les distances associees");
}

CT_VirtualAbstractStep* StepComputeDensityAndDistancesMultiScan::createNewInstance(CT_StepInitializeData &dataInit)
{
    // Creates an instance of this step
    return new StepComputeDensityAndDistancesMultiScan(dataInit);
}

void StepComputeDensityAndDistancesMultiScan::createInResultModelListProtected()
{
    // results with Scenes
    CT_InOneOrMoreGroupModel *group = new CT_InOneOrMoreGroupModel();

    CT_InStandardItemDrawableModel *item = new CT_InStandardItemDrawableModel(DEF_SearchInScene,
                                                                              CT_Scene::staticGetType(),
                                                                              tr("ScËne"));

    group->addItem(item);

    CT_InResultModelGroup *resultModel = new CT_InResultModelGroup(DEF_SearchInResult, group, tr("ScËne(s)"));

    addInResultModel(resultModel);


    // results with Scanners
    CT_InOneOrMoreGroupModel *group2 = new CT_InOneOrMoreGroupModel();

    CT_InStandardItemDrawableModel *item2 = new CT_InStandardItemDrawableModel(DEF_SearchInScanner,
                                                                              CT_Scanner::staticGetType(),
                                                                              tr("Scanner"));

    group2->addItem(item2);

    CT_InResultModelGroup *resultModel2 = new CT_InResultModelGroup(DEF_SearchInResultScanner, group, tr("Scanner(s)"));

    addInResultModel(resultModel2);
}

void StepComputeDensityAndDistancesMultiScan::createOutResultModelListProtected()
{
    // Compute number of categories
    _nCategories = computeGridTools::getNCategories( _categoriesMarks );

    if ( _nCategories > 1 )
    {
        for ( int i = 0 ; i < _nCategories ; i++ )
        {
            addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultList(), QList<CT_AbstractItemDrawable*>() << new CT_RegularGridDouble(), QString(QString("CT_RegularGridDistOutCat").append(QString::number(i))) ));
            addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultList(), QList<CT_AbstractItemDrawable*>() << new CT_RegularGridDouble(), QString(QString("CT_RegularGridDistInCat").append(QString::number(i))) ));
            addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultList(), QList<CT_AbstractItemDrawable*>() << new CT_RegularGridInt(), QString(QString("CT_RegularGridHitsCat").append(QString::number(i))) ));
        }
    }

    addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultList(), QList<CT_AbstractItemDrawable*>() << new CT_RegularGridInt(), "DeltaOutGrid"));
    addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultList(), QList<CT_AbstractItemDrawable*>() << new CT_RegularGridInt(), "DeltaInGrid"));
    addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultList(), QList<CT_AbstractItemDrawable*>() << new CT_RegularGridInt(), "DeltaBefore"));
    addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultList(), QList<CT_AbstractItemDrawable*>() << new CT_RegularGridInt(), "DeltaTheoritical"));

    addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultList(), QList<CT_AbstractItemDrawable*>() << new CT_RegularGridInt(), "CT_ResultRegularGridBefore"));
    addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultList(), QList<CT_AbstractItemDrawable*>() << new CT_RegularGridInt(), "CT_ResultRegularGridTheoritical"));
    addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultList(), QList<CT_AbstractItemDrawable*>() << new CT_RegularGridInt(), "CT_ResultRegularGridHits"));

    addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultList(), QList<CT_AbstractItemDrawable*>() << new CT_RegularGridDouble(), "CT_ResultRegularGridDensity"));

    addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultGroup(), QList<CT_AbstractItemDrawable*>() << new CT_RegularGridInt(), "CT_ResultBestPointOfView"));
}

void StepComputeDensityAndDistancesMultiScan::createConfigurationDialog()
{
    _configDialog = new CT_StepConfigurableDialog();
    _configDialog->setStep(*this);

    _configDialog->addDouble(tr("Size of a voxel"),tr("meters"),0.0001,10000,2, _res );
    _configDialog->addDouble(tr("Minimum intensity taken in account"),tr(""),-100000,100000,2, _intensityThresh );
    _configDialog->addBool("Consider the threshold as a minimum value","","",_greaterThanThresh);
    _configDialog->addDouble(tr("Minimum number of effective ray in a voxel to take it into account"),tr(""),-100000,100000,2, _effectiveRayThresh );

    _configDialog->addInt(tr("Benchmark between first and second category"), tr(""), -10000, 1000000, _categoriesMarks[0]);
    _configDialog->addInt(tr("Benchmark between second and thirs category"), tr(""), -10000, 1000000, _categoriesMarks[1]);
    _configDialog->addInt(tr("Benchmark between third and fourth category"), tr(""), -10000, 1000000, _categoriesMarks[2]);
    _configDialog->addInt(tr("Benchmark between fourth and fifth category"), tr(""), -10000, 1000000, _categoriesMarks[3]);

    _configDialog->addBool("Save each result in \".vox\" format and the merged result as \".mox\"","","",_saveGrid);
}

void StepComputeDensityAndDistancesMultiScan::compute()
{
    // Get the input file name and path
    QString filePathAndName = (dynamic_cast<CT_VirtualAbstractStep*> (this->parentStep()))->getAllSettings().at(0).toString();
    QFileInfo fileInfo ( filePathAndName );
    _inputInFilePath = fileInfo.absolutePath()+QString("/");
    _inputInFileName = fileInfo.baseName();

    // Get the input scenes and scanners
    const QList<Result*> &inResultList = getInputResults();
    CT_ResultList* inputScenesList = dynamic_cast<CT_ResultList*>(inResultList.at(0));
    CT_ResultList* inputScannersList = dynamic_cast<CT_ResultList*>(inResultList.at(1));

    // Make sure the lists have the same number of items
    assert ( inputScenesList->getNumberItemDrawable() == inputScannersList->getNumberItemDrawable() );

    // Get the number of scanners and scenes
    int numberOfItems = inputScenesList->getNumberItemDrawable();

    // Colorise scanners with the same color than the future bestViewGrid
    float scaling = 240.0/(numberOfItems-1);
    for ( unsigned int i = 0 ; i < numberOfItems ; i++ )
    {
        CT_Scanner*   currentScanner = dynamic_cast<CT_Scanner*>(inputScannersList->getItemDrawable(i));
        currentScanner->setColor( QColor::fromHsv( (float)i*scaling, 255, 255 ) );
    }

    // Get the future results
    CT_ResultList*  outputResultDeltaOutGrids;
    CT_ResultList*  outputResultDeltaInGrids;
    CT_ResultList*  outputResultDeltaBeforeGrids;
    CT_ResultList*  outputResultDeltaTheoriticalGrids;
    CT_ResultList*  outputResultBeforeGrids;
    CT_ResultList*  outputResultTheoriticalGrids;
    CT_ResultList*  outputResultHitsGrids;
    CT_ResultList*  outputResultDensityGrids;
    CT_ResultGroup*   outputResultBestViewGrid;

    if ( _nCategories > 1 )
    {
        outputResultDeltaOutGrids = dynamic_cast<CT_ResultList*>(getOutResultList().at(3*_nCategories));
        outputResultDeltaInGrids = dynamic_cast<CT_ResultList*>(getOutResultList().at(3*_nCategories+1));
        outputResultDeltaBeforeGrids = dynamic_cast<CT_ResultList*>(getOutResultList().at(3*_nCategories+2));
        outputResultDeltaTheoriticalGrids = dynamic_cast<CT_ResultList*>(getOutResultList().at(3*_nCategories+3));

        outputResultBeforeGrids = dynamic_cast<CT_ResultList*>(getOutResultList().at(3*_nCategories+4));
        outputResultTheoriticalGrids = dynamic_cast<CT_ResultList*>(getOutResultList().at(3*_nCategories+5));
        outputResultHitsGrids = dynamic_cast<CT_ResultList*>(getOutResultList().at(3*_nCategories+6));
        outputResultDensityGrids = dynamic_cast<CT_ResultList*>(getOutResultList().at(3*_nCategories+7));
        outputResultBestViewGrid = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(3*_nCategories+8));
    }

    else
    {
        outputResultDeltaOutGrids = dynamic_cast<CT_ResultList*>(getOutResultList().at(0));
        outputResultDeltaInGrids = dynamic_cast<CT_ResultList*>(getOutResultList().at(1));
        outputResultDeltaBeforeGrids = dynamic_cast<CT_ResultList*>(getOutResultList().at(2));
        outputResultDeltaTheoriticalGrids = dynamic_cast<CT_ResultList*>(getOutResultList().at(3));

        outputResultBeforeGrids = dynamic_cast<CT_ResultList*>(getOutResultList().at(4));
        outputResultTheoriticalGrids = dynamic_cast<CT_ResultList*>(getOutResultList().at(5));
        outputResultHitsGrids = dynamic_cast<CT_ResultList*>(getOutResultList().at(6));
        outputResultDensityGrids = dynamic_cast<CT_ResultList*>(getOutResultList().at(7));
        outputResultBestViewGrid = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(8));
    }

    // Declaring the local resulting grids lists
    QList< CT_RegularGridDouble* >      resultingDeltaOutGrids;
    QList< CT_RegularGridDouble* >      resultingDeltaInGrids;
    QList< CT_RegularGridDouble* >      resultingDeltaBeforeGrids;
    QList< CT_RegularGridDouble* >      resultingDeltaTheoriticalGrids;
    QList< CT_RegularGridInt* >         resultingBeforeGrids;
    QList< CT_RegularGridInt* >         resultingTheoriticalGrids;
    QList< CT_RegularGridInt* >         resultingHitsGrids;
    QList< CT_RegularGridDouble* >      resultingDensityGrids;

    // These lists represent : list that contains for each category the hit number from the scanner i (the hit number from the scanner j of the category j = resultingCategoryHitGridsListList[j][i])
    QList< QList<CT_RegularGridInt*> >      resultingCategoryHitGridsListList;
    QList< QList<CT_RegularGridDouble*> >   resultingCategoryDeltaInGridsListList;
    QList< QList<CT_RegularGridDouble*> >   resultingCategoryDeltaOutGridsListList;

    // Push back as many new qlists as number of categories
    if ( _nCategories > 1 )
    {
        for ( int i = 0 ; i < _nCategories ; i++ )
        {
            resultingCategoryHitGridsListList.push_back( QList<CT_RegularGridInt*>() );
            resultingCategoryDeltaInGridsListList.push_back( QList<CT_RegularGridDouble*>() );
            resultingCategoryDeltaOutGridsListList.push_back( QList<CT_RegularGridDouble*>() );
        }
    }

    // Getting the bounding box of the scenes (should be all the same)
    const CT_Scene* calibrationScene = dynamic_cast<CT_Scene*>(inputScenesList->getItemDrawable(0));
    CT_Point bot = createCtPoint( calibrationScene->xMin(), calibrationScene->yMin(), calibrationScene->zMin() );
    CT_Point top = createCtPoint( calibrationScene->xMax(), calibrationScene->yMax(), calibrationScene->zMax() );

    // For each couple of (scene,scan)
    for ( int i = 0 ; i < numberOfItems ; i++ )
    {
        // Getting the current scene and scanner
        const CT_Scene*     currentScene = dynamic_cast<CT_Scene*>(inputScenesList->getItemDrawable(i));
        const CT_Scanner*   currentScanner = dynamic_cast<CT_Scanner*>(inputScannersList->getItemDrawable(i));

        // Computing the different grids
        CT_RegularGridDouble*   currentDensityGrid = NULL;
        CT_RegularGridInt*      currentHitGrid = NULL;
        CT_RegularGridInt*      currentTheoriticalGrid = NULL;
        CT_RegularGridInt*      currentBeforeGrid = NULL;
        CT_RegularGridDouble*   currentDeltaTheoriticalGrid = NULL;
        CT_RegularGridDouble*   currentDeltaBeforeGrid = NULL;
        CT_RegularGridDouble*   currentDeltaDeltaInGrid = NULL;
        CT_RegularGridDouble*   currentDeltaDeltaOutGrid = NULL;

        QList< CT_RegularGridInt* >     currentCategoryHitsGridList;
        QList< CT_RegularGridDouble* >  currentCategoryDeltaInGridList;
        QList< CT_RegularGridDouble* >  currentCategoryDeltaOutGridList;

        if ( _nCategories > 1 )
        {
            // Filling the lists
            for ( int i = 0 ; i < _nCategories ; i++ )
            {
                currentCategoryHitsGridList.push_back( NULL );
                currentCategoryDeltaInGridList.push_back(NULL);
                currentCategoryDeltaOutGridList.push_back(NULL);
            }
        }

        if ( _nCategories > 1 )
            computeGridTools::computeHitGridAndDistancesAndCategories( bot, top, _res, currentScene->getPointCloud(), *currentScanner, _intensityThresh, _greaterThanThresh, 4, outputResultHitsGrids, currentHitGrid, 5, outputResultDeltaInGrids, currentDeltaDeltaInGrid, 6, outputResultDeltaOutGrids, currentDeltaDeltaOutGrid, currentCategoryHitsGridList, currentCategoryDeltaInGridList, currentCategoryDeltaOutGridList, _nCategories, _categoriesMarks );
        else
            computeGridTools::computeHitGridAndDistances( bot, top, _res, currentScene->getPointCloud(), *currentScanner, _intensityThresh, _greaterThanThresh, 4, outputResultHitsGrids, currentHitGrid, 5, outputResultDeltaInGrids, currentDeltaDeltaInGrid, 6, outputResultDeltaOutGrids, currentDeltaDeltaOutGrid );

        computeGridTools::computeTheoriticalGridAndDistances( bot, top, _res, *currentScanner, 0, outputResultTheoriticalGrids, currentTheoriticalGrid, 1, outputResultDeltaTheoriticalGrids, currentDeltaTheoriticalGrid );
        computeGridTools::computeBeforeGridAndDistances( bot, top, _res, currentScene->getPointCloud(), *currentScanner, 2, outputResultBeforeGrids, currentBeforeGrid, 3, outputResultDeltaBeforeGrids, currentDeltaBeforeGrid );

        currentDensityGrid = computeGridTools::computeDensityGrid(7, outputResultDensityGrids, currentHitGrid, currentTheoriticalGrid, currentBeforeGrid, _effectiveRayThresh );

        // Setting grids results
        if ( _nCategories > 1 )
        {
            for ( unsigned int j = 0 ; j < _nCategories ; j++ )
            {
                // On les attache au resultat de leur categorie
                currentCategoryHitsGridList[j]->changeResult( dynamic_cast<CT_ResultList*>(getOutResultList().at(3*j)) );
                currentCategoryDeltaInGridList[j]->changeResult( dynamic_cast<CT_ResultList*>(getOutResultList().at(3*j+1)) );
                currentCategoryDeltaOutGridList[j]->changeResult( dynamic_cast<CT_ResultList*>(getOutResultList().at(3*j+2)) );

                // Adding the lists of grids to the corresponding list
                resultingCategoryHitGridsListList[j].push_back( currentCategoryHitsGridList[j] );
                resultingCategoryDeltaOutGridsListList[j].push_back( currentCategoryDeltaInGridList[j] );
                resultingCategoryDeltaInGridsListList[j].push_back( currentCategoryDeltaOutGridList[j] );
            }
        }

        // Adding the grids to the corresponding list
        resultingDensityGrids.push_back( currentDensityGrid );
        resultingHitsGrids.push_back( currentHitGrid );
        resultingTheoriticalGrids.push_back( currentTheoriticalGrid );
        resultingBeforeGrids.push_back( currentBeforeGrid );
        resultingDeltaTheoriticalGrids.push_back( currentDeltaTheoriticalGrid );
        resultingDeltaBeforeGrids.push_back( currentDeltaBeforeGrid );
        resultingDeltaInGrids.push_back( currentDeltaDeltaInGrid );
        resultingDeltaOutGrids.push_back( currentDeltaDeltaOutGrid );
    }

    // Computing the best point of view grid
    CT_RegularGridInt*  bestPointOfViewGrid = computeGridTools::computeBestPointOfViewGrid(0, outputResultBestViewGrid, bot, top, _res, resultingTheoriticalGrids, resultingBeforeGrids, resultingHitsGrids );

    // Setting the output lists
    outputResultDensityGrids->setList( (QList<CT_AbstractItemDrawable*>&)resultingDensityGrids );
    outputResultHitsGrids->setList( (QList<CT_AbstractItemDrawable*>&)resultingHitsGrids );
    outputResultTheoriticalGrids->setList( (QList<CT_AbstractItemDrawable*>&)resultingTheoriticalGrids );
    outputResultBeforeGrids->setList( (QList<CT_AbstractItemDrawable*>&)resultingBeforeGrids );
    outputResultDeltaTheoriticalGrids->setList( (QList<CT_AbstractItemDrawable*>&)resultingDeltaTheoriticalGrids );
    outputResultDeltaBeforeGrids->setList( (QList<CT_AbstractItemDrawable*>&)resultingDeltaBeforeGrids );
    outputResultDeltaInGrids->setList( (QList<CT_AbstractItemDrawable*>&)resultingDeltaInGrids );
    outputResultDeltaOutGrids->setList( (QList<CT_AbstractItemDrawable*>&)resultingDeltaOutGrids );

    outputResultBestViewGrid->setItemDrawable( bestPointOfViewGrid );

    if ( _nCategories > 1 )
    {
        for ( int i = 0 ; i < _nCategories; i++ )
        {
            dynamic_cast<CT_ResultList*>(getOutResultList().at(3*i))->setList( (QList<CT_AbstractItemDrawable*>&)resultingCategoryDeltaOutGridsListList[i] );
            dynamic_cast<CT_ResultList*>(getOutResultList().at(3*i+1))->setList( (QList<CT_AbstractItemDrawable*>&)resultingCategoryDeltaInGridsListList[i] );
            dynamic_cast<CT_ResultList*>(getOutResultList().at(3*i+2))->setList( (QList<CT_AbstractItemDrawable*>&)resultingCategoryHitGridsListList[i] );
        }
    }

    // Save the results if demanded
    if ( _saveGrid )
    {
        computeGridTools::saveDensityAndDistancesResult( resultingDensityGrids, resultingHitsGrids, resultingTheoriticalGrids, resultingBeforeGrids, resultingDeltaTheoriticalGrids, resultingDeltaBeforeGrids, resultingDeltaInGrids, resultingDeltaOutGrids, resultingCategoryHitGridsListList, resultingCategoryDeltaInGridsListList, resultingCategoryDeltaOutGridsListList, bestPointOfViewGrid, _nCategories, _categoriesMarks, _inputInFilePath, _inputInFileName );
    }
}
