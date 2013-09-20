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

#include "stepcomputedensityonlymultiscan.h"

#include "ct_itemdrawable/model/inModel/ct_inoneormoregroupmodel.h"
#include "ct_itemdrawable/model/outModel/ct_outstandardgroupmodel.h"
#include "ct_itemdrawable/model/inModel/ct_instandarditemdrawablemodel.h"
#include "ct_itemdrawable/model/outModel/ct_outstandarditemdrawablemodel.h"

#include "ct_result/model/inModel/ct_inresultmodelgroup.h"
#include "ct_result/model/outModel/ct_outresultmodelgroup.h"

#include "ct_result/ct_resultgroup.h"
#include "ct_itemdrawable/ct_scene.h"

#include "ct_view/ct_stepconfigurabledialog.h"                  // For the configuration dialog window

#include "algorithmewoo.h"                           // Raytracing traversal algorithm
#include "abstractvisitorraytracing.h"               // The visitors of the raytracing
#include "visitorraytracingincrement.h"              // Increment visitors

#include <QFileInfo>                                            //
#include <QDebug>                                               // Some printing output in debug mode

#define DEF_SearchInResult "r"
#define DEF_SearchInScene   "sc"
#define DEF_SearchInResultScanner "rs"
#define DEF_SearchInScanner   "sca"

StepComputeDensityOnlyMultiScan::StepComputeDensityOnlyMultiScan(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _res = 0.5;
    _intensityThresh = 10;
    _greaterThanThresh = true;

    _effectiveRayThresh = 10;

    _nCategories = 1;
    _categoriesMarks.resize( 4, -1 );
    _saveGrid = true;
}

QString StepComputeDensityOnlyMultiScan::getStepDescription() const
{
    // Gives the descrption to print in the GUI
    return tr("Discretise l'espace en voxels et calcule un indice de denste par voxel");
}

CT_VirtualAbstractStep* StepComputeDensityOnlyMultiScan::createNewInstance(CT_StepInitializeData &dataInit)
{
    // Creates an instance of this step
    return new StepComputeDensityOnlyMultiScan(dataInit);
}

void StepComputeDensityOnlyMultiScan::createInResultModelListProtected()
{
    // results with Scenes
    CT_InOneOrMoreGroupModel *group = new CT_InOneOrMoreGroupModel();

    CT_InStandardItemDrawableModel *item = new CT_InStandardItemDrawableModel(DEF_SearchInScene,
                                                                              CT_Scene::staticGetType(),
                                                                              tr("Scène"));

    group->addItem(item);

    CT_InResultModelGroup *resultModel = new CT_InResultModelGroup(DEF_SearchInResult, group, tr("Scène(s)"));

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

void StepComputeDensityOnlyMultiScan::createOutResultModelListProtected()
{
    // Compute number of categories
    _nCategories = computeGridTools::getNCategories( _categoriesMarks );

    if ( _nCategories > 1 )
    {
        for ( unsigned int i = 0 ; i < _nCategories ; i++ )
        {
            addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultList(), QList<CT_AbstractItemDrawable*>() << new CT_RegularGridInt(), QString("CT_ResultRegularGridHitsCategory").append(QString().number(i)) ) );
        }
    }

    addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultList(), QList<CT_AbstractItemDrawable*>() << new CT_RegularGridInt(), "BeforeGrids"));
    addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultList(), QList<CT_AbstractItemDrawable*>() << new CT_RegularGridInt(), "TheoriticalGrids"));
    addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultList(), QList<CT_AbstractItemDrawable*>() << new CT_RegularGridInt(), "HitsGrids"));

    addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultList(), QList<CT_AbstractItemDrawable*>() << new CT_RegularGridDouble(), "DensityGrids"));

    addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultGroup(), QList<CT_AbstractItemDrawable*>() << new CT_RegularGridInt(), "BestViewGrid"));
}

void StepComputeDensityOnlyMultiScan::createConfigurationDialog()
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

    _configDialog->addBool("Save results (one \"LVox Density File\" (.ldf) file per scan and one \"Merged Density File\" (.mdf) )","","",_saveGrid);
}

void StepComputeDensityOnlyMultiScan::compute()
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
        currentScanner->setColor( QColor::fromHsv( i*scaling, 255, 255 ) );
    }

    // Get the future results
    CT_ResultList*  outputResultBeforeGrids;
    CT_ResultList*  outputResultTheoriticalGrids;
    CT_ResultList*  outputResultHitsGrids;
    CT_ResultList*  outputResultDensityGrids;
    CT_ResultGroup*   outputResultBestViewGrid;

    if ( _nCategories > 1 )
    {
        outputResultBeforeGrids = dynamic_cast<CT_ResultList*>(getOutResultList().at(_nCategories));
        outputResultTheoriticalGrids = dynamic_cast<CT_ResultList*>(getOutResultList().at(_nCategories+1));
        outputResultHitsGrids = dynamic_cast<CT_ResultList*>(getOutResultList().at(_nCategories+2));
        outputResultDensityGrids = dynamic_cast<CT_ResultList*>(getOutResultList().at(_nCategories+3));
        outputResultBestViewGrid = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(_nCategories+4));
    }

    else
    {
        outputResultBeforeGrids = dynamic_cast<CT_ResultList*>(getOutResultList().at(0));
        outputResultTheoriticalGrids = dynamic_cast<CT_ResultList*>(getOutResultList().at(1));
        outputResultHitsGrids = dynamic_cast<CT_ResultList*>(getOutResultList().at(2));
        outputResultDensityGrids = dynamic_cast<CT_ResultList*>(getOutResultList().at(3));
        outputResultBestViewGrid = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(4));
    }

    // Declaring the local resulting grids lists
    QList< CT_RegularGridInt* >         resultingHitsGrids;
    QList< CT_RegularGridInt* >         resultingTheoriticalGrids;
    QList< CT_RegularGridInt* >         resultingBeforeGrids;
    QList< CT_RegularGridDouble* >      resultingDensityGrids;

    // These lists represent : list that contains for each category the hit number from the scanner i (the hit number from the scanner j of the category j = resultingCategoryHitGridsListList[j][i])
    QList< QList<CT_RegularGridInt*> >      resultingCategoryHitGridsListList;

    // Push back as many new qlists as number of categories
    if ( _nCategories > 1 )
    {
        for ( int i = 0 ; i < _nCategories ; i++ )
        {
            resultingCategoryHitGridsListList.push_back( QList<CT_RegularGridInt*>() );
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
        CT_RegularGridInt*      currentMainHitsGrid = computeGridTools::computeHitGrid(0, outputResultHitsGrids, bot, top, _res, currentScene->getPointCloud(), _intensityThresh, _greaterThanThresh );
        CT_RegularGridInt*      currentTheoriticalGrid = computeGridTools::computeTheoriticalGrid(1, outputResultTheoriticalGrids, bot, top, _res, *currentScanner );
        CT_RegularGridInt*      currentBeforeGrid = computeGridTools::computeBeforeGrid(2, outputResultBeforeGrids, bot, top, _res, currentScene->getPointCloud(), *currentScanner );
        CT_RegularGridDouble*   currentDensityGrid = computeGridTools::computeDensityGrid(3, outputResultDensityGrids, currentMainHitsGrid, currentTheoriticalGrid, currentBeforeGrid, _effectiveRayThresh );

        // Declaring a list of grids in case categories are demanded
        QList< CT_RegularGridInt* > currentCategoryHitsGridList;
        QList< qint64 >             idList;
        QList< Result* >            resultList;

        // Computing the category hit grids is done only if there is several categories
        if ( _nCategories > 1 )
        {
            // Filling the list with as many NULL pointer as the number of categories and set the appropriate id and result
            for ( unsigned int j = 0 ; j < _nCategories ; j++ )
            {
                currentCategoryHitsGridList.push_back( NULL );
                idList.push_back(4+j);

                // On l'attache a la jeme categorie scanner
                resultList.push_back( dynamic_cast<CT_ResultList*>(getOutResultList().at(i)) );
            }

            computeGridTools::computeCategoriesHitGrids(bot, top, _res, currentScene->getPointCloud(), _intensityThresh, _greaterThanThresh, currentCategoryHitsGridList, idList, resultList, _nCategories, _categoriesMarks );

            for ( unsigned int j = 0 ; j < _nCategories ; j++ )
            {
                // Setting the result
                currentCategoryHitsGridList[j]->changeResult( dynamic_cast<CT_ResultGroup*>(getOutResultList().at(i)) );

                // Adding the lists of grids to the corresponding list
                resultingCategoryHitGridsListList[j].push_back( currentCategoryHitsGridList[j] );
            }
        }

        // Setting grids results
        currentMainHitsGrid->changeResult( outputResultHitsGrids );
        currentTheoriticalGrid->changeResult( outputResultTheoriticalGrids );
        currentBeforeGrid->changeResult( outputResultBeforeGrids );
        currentDensityGrid->changeResult( outputResultDensityGrids );

        // Adding the grids to the corresponding list
        resultingHitsGrids.push_back( currentMainHitsGrid );
        resultingTheoriticalGrids.push_back( currentTheoriticalGrid );
        resultingBeforeGrids.push_back( currentBeforeGrid );
        resultingDensityGrids.push_back( currentDensityGrid );
    }

    // Computing the best point of view grid
    CT_RegularGridInt*  bestPointOfViewGrid = computeGridTools::computeBestPointOfViewGrid(0, outputResultBestViewGrid, bot, top, _res, resultingTheoriticalGrids, resultingBeforeGrids, resultingHitsGrids );

    // Setting the output lists
    outputResultHitsGrids->setList( (QList<CT_AbstractItemDrawable*>&)resultingHitsGrids );
    outputResultTheoriticalGrids->setList( (QList<CT_AbstractItemDrawable*>&)resultingTheoriticalGrids );
    outputResultBeforeGrids->setList( (QList<CT_AbstractItemDrawable*>&)resultingBeforeGrids );
    outputResultDensityGrids->setList( (QList<CT_AbstractItemDrawable*>&)resultingDensityGrids );

    outputResultBestViewGrid->setItemDrawable( bestPointOfViewGrid );

    if ( _nCategories > 1 )
    {
        for ( int i = 0 ; i < _nCategories; i++ )
        {
            dynamic_cast<CT_ResultList*>(getOutResultList().at(i))->setList( (QList<CT_AbstractItemDrawable*>&)resultingCategoryHitGridsListList[i] );
        }
    }

    // Save the results if demanded
    if ( _saveGrid )
    {
        computeGridTools::saveDensityOnlyResult( resultingDensityGrids, resultingHitsGrids, resultingTheoriticalGrids, resultingBeforeGrids, resultingCategoryHitGridsListList, bestPointOfViewGrid, _nCategories, _categoriesMarks, _inputInFilePath, _inputInFileName );
    }

    resultingTheoriticalGrids.at(0)->saveAsText("ResultatMultiPointDeVue0.txt");
}
