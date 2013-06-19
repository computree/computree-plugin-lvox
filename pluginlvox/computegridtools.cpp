#include "computegridtools.h"

#include "ct_math/ct_mathpoint.h"           // Calcul de distance point a point

#include <QDateTime>
#include <QFile>

int computeGridTools::getNCategories(vector<int> &benchmarks)
{
    // Get the number of categories
    int nCategories = 1;

    int i = 0;
    while ( benchmarks[i] > 0 )
    {
        nCategories++;
        i++;
    }

    return nCategories;
}

CT_RegularGridInt* computeGridTools::computeHitGrid( qint64 id, Result *result, const CT_Point &bot, const CT_Point &top, float res, const CT_AbstractPointCloud* inputCloud, double intensityThresh, bool greaterThan)
{
    CT_RegularGridInt* grilleHits = new CT_RegularGridInt(NULL, id, result, top, bot, res, 0);
    float cloudSize = inputCloud->cloudSize();
    CT_Point currentVoxel;

    bool considerHit;

    // For each point from the input scene add it to the correct voxel
    for ( unsigned int i = 0 ; i < cloudSize ; i++ )
    {
        // Only update the grid if the intensity is greater than the threshold
        greaterThan == true ? considerHit = ((*inputCloud)[i].intensity > intensityThresh) : considerHit = ((*inputCloud)[i].intensity < intensityThresh);
        if ( considerHit )
        {
            // Get the voxel containing the point
            if ( !grilleHits->spaceToGrid( (*inputCloud)[i], currentVoxel ) )
            {
                qDebug() << "Le point "<< i << " de la scene n'est pas dans la grille";
                exit(0);
            }

            // Increment the data of the voxel
            grilleHits->setData( currentVoxel, grilleHits->getData( currentVoxel ) + 1 );
        }
    }

    // Calculate min and max in order to view the grid as a colored map later
    grilleHits->calculateMinMax();

    return grilleHits;
}

void computeGridTools::computeCategoriesHitGrids( const CT_Point& bot, const CT_Point& top, float res, const CT_AbstractPointCloud* inputCloud, double intensityThresh, bool greaterThan, QList< CT_RegularGridInt* >& categoryHitGridList, QList< qint64 > idList, QList< Result* > resultList, int nCategories, vector<int> &benchmarks )
{
    // Make sure parameters are valid
    assert ( categoryHitGridList.size() == nCategories && idList.size() == nCategories && resultList.size() == nCategories );
    for ( unsigned int i = 0 ; i < nCategories ; i++ )
        assert ( categoryHitGridList[i] == NULL );

    // Allocating the output category hit grids
    for ( unsigned int i = 0 ; i < nCategories ; i++ )
    {
        categoryHitGridList[i] = new CT_RegularGridInt(NULL, idList[i], resultList[i], top, bot, res, 0 );
    }

    // Declaring as many possible variables outside the loop to reduce allocation time
    CT_Point currentVoxel;
    int currentCategory;
    unsigned int cloudSize = inputCloud->cloudSize();
    bool considerHit;

    // For each point from the input scene add it to the right voxel from the right grid
    for ( unsigned int i = 0 ; i < cloudSize ; i++ )
    {
        // Only update the grid if the intensity is greater than the threshold
        greaterThan == true ? considerHit = ((*inputCloud)[i].intensity > intensityThresh) : considerHit = ((*inputCloud)[i].intensity < intensityThresh);
        if ( considerHit )
        {
            // Get the voxel containing the point
            if ( !categoryHitGridList[0]->spaceToGrid( (*inputCloud)[i], currentVoxel ) )
            {
                qDebug() << "Le point "<< i << " de la scene n'est pas dans la grille";
                exit(0);
            }

            // Get the category of the current hit
            currentCategory = getCategory( (*inputCloud)[i].intensity, nCategories, benchmarks );

            // Increment the data of the voxel in the right grid
            categoryHitGridList[currentCategory]->setData( currentVoxel, categoryHitGridList[currentCategory]->getData( currentVoxel ) + 1 );
        }
    }

    // Calculate min and max in order to view the grid as a colored map later
    for ( unsigned int i = 0 ; i < nCategories ; i++ )
    {
        categoryHitGridList[i]->calculateMinMax();
    }
}

CT_RegularGridInt* computeGridTools::computeAllHitGrids(qint64 id, Result *result, const CT_Point &bot, const CT_Point &top, float res, const CT_AbstractPointCloud *inputCloud, double intensityThresh, bool greaterThan, QList<CT_RegularGridInt *> &categoryHitGridList, QList<qint64> idList, QList<Result *> resultList, int nCategories, vector<int> &benchmarks)
{
    // Make sure parameters are valid
    assert ( categoryHitGridList.size() == nCategories && idList.size() == nCategories && resultList.size() == nCategories );
    for ( unsigned int i = 0 ; i < nCategories ; i++ )
        assert ( categoryHitGridList[i] == NULL );

    // Allocating the output category hit grids
    CT_RegularGridInt* grilleHits = new CT_RegularGridInt(NULL, id, result, top, bot, res, 0);
    for ( unsigned int i = 0 ; i < nCategories ; i++ )
    {
        categoryHitGridList[i] = new CT_RegularGridInt(NULL, idList[i], resultList[i], top, bot, res, 0 );
    }

    // Declaring as many possible variables outside the loop to reduce allocation time
    CT_Point currentVoxel;
    int currentCategory;
    unsigned int cloudSize = inputCloud->cloudSize();
    bool considerHit;

    // For each point from the input scene add it to the right voxel from the right grid
    for ( unsigned int i = 0 ; i < cloudSize ; i++ )
    {
        // Only update the grid if the intensity is greater than the threshold
        greaterThan == true ? considerHit = ((*inputCloud)[i].intensity > intensityThresh) : considerHit = ((*inputCloud)[i].intensity < intensityThresh);
        if ( considerHit )
        {
            // Get the voxel containing the point
            if ( !categoryHitGridList[0]->spaceToGrid( (*inputCloud)[i], currentVoxel ) )
            {
                qDebug() << "Le point "<< i << " de la scene n'est pas dans la grille";
                exit(0);
            }

            // Get the category of the current hit
            currentCategory = getCategory( (*inputCloud)[i].intensity, nCategories, benchmarks );

            // Increment the data of the voxel in the right category grid AND in the main grid
            categoryHitGridList[currentCategory]->setData( currentVoxel, categoryHitGridList[currentCategory]->getData( currentVoxel ) + 1 );
            grilleHits->setData( currentVoxel, grilleHits->getData( currentVoxel ) + 1 );
        }
    }

    // Calculate min and max in order to view the grid as a colored map later
    for ( unsigned int i = 0 ; i < nCategories ; i++ )
    {
        categoryHitGridList[i]->calculateMinMax();
    }

    grilleHits->calculateMinMax();

    return grilleHits;
}

void computeGridTools::computeHitGridAndDistances(const CT_Point &bot,
                                                  const CT_Point& top,
                                                  float res,
                                                  const CT_AbstractPointCloud *inputCloud,
                                                  const CT_Scanner &scanner,
                                                  double intensityThresh,
                                                  bool greaterThan,
                                                  qint64 idHit,
                                                  Result* resultHit,
                                                  CT_RegularGridInt *&outputHitGrid,
                                                  qint64 idDeltaIn,
                                                  Result* resultDeltaIn,
                                                  CT_RegularGridDouble *&outputDeltaInGrid,
                                                  qint64 idDeltaOut,
                                                  Result* resultDeltaOut,
                                                  CT_RegularGridDouble *&outputDeltaOutGrid)
{
    // Make sure parameters are good
    assert ( outputHitGrid == NULL && outputDeltaInGrid == NULL && outputDeltaOutGrid == NULL );

    // Allocate results
    outputHitGrid = new CT_RegularGridInt(NULL, idHit, resultHit, top, bot, res, 0);
    outputDeltaInGrid = new CT_RegularGridDouble (NULL, idDeltaIn, resultDeltaIn, top, bot, res, 0 );
    outputDeltaOutGrid = new CT_RegularGridDouble (NULL, idDeltaOut, resultDeltaOut, top, bot, res, 0 );

    // Declaring as many as possible variables outside the loop
    float cloudSize = inputCloud->cloudSize();
    CT_Point currentVoxel;
    CT_Point currentHitPoint;
    bool considerHit;
    CT_Point in, out, voxelBot, voxelTop;

    // For each point from the input scene add it to the correct voxel
    for ( unsigned int i = 0 ; i < cloudSize ; i++ )
    {
        currentHitPoint = (*inputCloud)[i];

        greaterThan == true ? considerHit = (currentHitPoint.intensity > intensityThresh) : considerHit = (currentHitPoint.intensity < intensityThresh);

        if ( considerHit )
        {
            // Get the voxel containing the point
            if ( !outputHitGrid->spaceToGrid( currentHitPoint, currentVoxel ) )
            {
                qDebug() << "Le point "<< i << " de la scene n'est pas dans la grille";
                exit(0);
            }

            // Get the index of the currentVoxel
            int currentVoxelID = outputHitGrid->gridToIndex( currentVoxel );

            // Increment the hit grid at the corresponding voxel
            outputHitGrid->setData( currentVoxel, outputHitGrid->getData( currentVoxel ) + 1 );

            // Getting the direction of the ray and creating it
            CT_Point direction;
            direction.x = currentHitPoint.x - scanner.getPosition().x();
            direction.y = currentHitPoint.y - scanner.getPosition().y();
            direction.z = currentHitPoint.z - scanner.getPosition().z();
            CT_Ray currentRay(NULL, 0, NULL, scanner.getPosition(), direction );

            // Getting the in and out points in the voxel and updating the distances grids
            outputHitGrid->getBBox( currentVoxelID, voxelBot, voxelTop );
            if ( currentRay.intersect( voxelBot, voxelTop, in, out ) )
            {
                outputDeltaInGrid->setData( currentVoxelID, outputDeltaInGrid->getData(currentVoxelID) + CT_MathPoint::distance3D( in, currentHitPoint ) );
                outputDeltaOutGrid->setData( currentVoxelID, outputDeltaOutGrid->getData(currentVoxelID) + CT_MathPoint::distance3D( out, currentHitPoint ) );
            }
        }
    }

    // To get the mean distance we have to divide in each voxel the sum of distances by the number of hits
    for ( unsigned int i = 0 ; i < outputHitGrid->nVoxels() ; i++ )
    {
        if ( outputHitGrid->getData(i) == 0 )
        {
            outputDeltaInGrid->setData(i,-1);
            outputDeltaOutGrid->setData(i,-1);
        }

        else
        {
            outputDeltaInGrid->setData(i, outputDeltaInGrid->getData(i)/outputHitGrid->getData(i) );
            outputDeltaOutGrid->setData(i, outputDeltaOutGrid->getData(i)/outputHitGrid->getData(i) );
        }
    }

    // Calculate min and max in order to view the grid as a colored map later
    outputHitGrid->calculateMinMax();
    outputDeltaInGrid->calculateMinMax();
    outputDeltaOutGrid->calculateMinMax();
}

void computeGridTools::computeHitGridAndDistancesAndCategories(const CT_Point &bot,
                                                               const CT_Point &top,
                                                               float res,
                                                               const CT_AbstractPointCloud *inputCloud,
                                                               const CT_Scanner &scanner,
                                                               double intensityThresh,
                                                               bool greaterThan,
                                                               qint64 idHit,
                                                               Result *resultHit,
                                                               CT_RegularGridInt *&outputHitGrid,
                                                               qint64 idDeltaIn,
                                                               Result *resultDeltaIn,
                                                               CT_RegularGridDouble *&outputDeltaInGrid,
                                                               qint64 idDeltaOut,
                                                               Result *resultDeltaOut,
                                                               CT_RegularGridDouble *&outputDeltaOutGrid,
                                                               QList<CT_RegularGridInt *> &categoryHitGridsList,
                                                               QList<CT_RegularGridDouble *> &categoryDeltaInGridsList,
                                                               QList<CT_RegularGridDouble *> &categoryDeltaOutGridsList,
                                                               int nCategories,
                                                               const vector<int> &categoriesBenchmarks)
{
    // Make sure parameters are good
    assert ( outputHitGrid == NULL && outputDeltaInGrid == NULL && outputDeltaOutGrid == NULL );
    assert ( categoryHitGridsList.size() == nCategories && categoryHitGridsList.size() == categoryDeltaInGridsList.size() && categoryHitGridsList.size() == categoryDeltaOutGridsList.size() );

    for ( int i = 0 ; i < nCategories ; i++ )
    {
        assert( categoryHitGridsList[i] == NULL && categoryDeltaInGridsList[i] == NULL && categoryDeltaOutGridsList[i] == NULL );
    }

    // Allocate results
    outputHitGrid = new CT_RegularGridInt(NULL, idHit, resultHit, top, bot, res, 0);
    outputDeltaInGrid = new CT_RegularGridDouble (NULL, idDeltaIn, resultDeltaIn, top, bot, res, 0 );
    outputDeltaOutGrid = new CT_RegularGridDouble (NULL, idDeltaOut, resultDeltaOut, top, bot, res, 0 );

    for ( int i = 0 ; i < nCategories ; i++ )
    {
        categoryHitGridsList[i] = new CT_RegularGridInt(NULL,0, NULL, top, bot, res, 0);
        categoryDeltaInGridsList[i] = new CT_RegularGridDouble(NULL,0, NULL, top, bot, res, 0);
        categoryDeltaOutGridsList[i] = new CT_RegularGridDouble(NULL,0, NULL, top, bot, res, 0);
    }

    // Declare each variable outside the loop in order to reduce allocating time
    float cloudSize = inputCloud->cloudSize();
    CT_Point currentVoxel;
    CT_Point currentHitPoint;
    bool considerHit;

    int currentVoxelID;
    int currentCategory;

    float currentDeltaIn;
    float currentDeltaOut;

    // For each point from the input scene add it to the correct voxel
    for ( unsigned int i = 0 ; i < cloudSize ; i++ )
    {
        // Get the next point and its category
        currentHitPoint = (*inputCloud)[i];

        greaterThan == true ? considerHit = (currentHitPoint.intensity > intensityThresh) : considerHit = (currentHitPoint.intensity < intensityThresh);
        if ( considerHit )
        {
            currentCategory = computeGridTools::getCategory( currentHitPoint.intensity, nCategories, categoriesBenchmarks );

            // Get the voxel containing the point
            if ( !outputHitGrid->spaceToGrid( currentHitPoint, currentVoxel ) )
            {
                qDebug() << "Le point "<< i << " de la scene n'est pas dans la grille";
                exit(0);
            }

            // Get the index of the currentVoxel
            currentVoxelID = outputHitGrid->gridToIndex( currentVoxel );

            // Increment the hit grid at the corresponding voxel
            outputHitGrid->setData( currentVoxel, outputHitGrid->getData( currentVoxel ) + 1 );

            // And the corresponding category hit grid data
            categoryHitGridsList[currentCategory]->setData( currentVoxel, categoryHitGridsList[currentCategory]->getData( currentVoxel ) + 1 );

            // Getting the direction of the ray and creating it
            CT_Point direction;
            direction.x = currentHitPoint.x - scanner.getPosition().x();
            direction.y = currentHitPoint.y - scanner.getPosition().y();
            direction.z = currentHitPoint.z - scanner.getPosition().z();
            CT_Ray currentRay(NULL,0, NULL, scanner.getPosition(), direction );

            // Getting the in and out points in the voxel and updating the distances grids
            CT_Point in, out;
            if ( currentRay.intersect( outputHitGrid, in, out ) )
            {
                // Computing distances
                currentDeltaIn = CT_MathPoint::distance3D( in, currentHitPoint );
                currentDeltaOut = CT_MathPoint::distance3D( out, currentHitPoint );

                // Updating the main grid
                outputDeltaInGrid->increment( currentVoxelID, currentDeltaIn );
                outputDeltaOutGrid->increment( currentVoxelID, currentDeltaOut );

                // And the category grid
                categoryDeltaInGridsList[currentCategory]->increment( currentVoxelID, currentDeltaIn );
                categoryDeltaOutGridsList[currentCategory]->increment( currentVoxelID, currentDeltaOut );
            }
        }
    }

    // To get the mean distance we have to divide in each voxel the sum of distances by the number of hits
    unsigned int nbVoxels = outputHitGrid->nVoxels();
    for ( unsigned int i = 0 ; i < nbVoxels ; i++ )
    {
        if ( outputHitGrid->getData(i) == 0 )
        {
            outputDeltaInGrid->setData(i,-1);
            outputDeltaOutGrid->setData(i,-1);
        }

        else
        {
            outputDeltaInGrid->setData(i, outputDeltaInGrid->getData(i)/outputHitGrid->getData(i) );
            outputDeltaOutGrid->setData(i, outputDeltaOutGrid->getData(i)/outputHitGrid->getData(i) );
        }
    }

    // To get the mean distance we have to divide in each voxel the sum of distances by the number of hits
    // For each category grid
    for ( int i = 0 ; i < nCategories ; i++ )
    {
        CT_RegularGridInt*      currentCategoryHitGridPtr = categoryHitGridsList[i];
        CT_RegularGridDouble*   currentCategoryDeltaInGridPtr = categoryDeltaInGridsList[i];
        CT_RegularGridDouble*   currentCategoryDeltaOutGridPtr = categoryDeltaOutGridsList[i];

        // For each voxel of the grid
        for ( unsigned int j = 0 ; j < nbVoxels ; j++ )
        {
            if ( currentCategoryHitGridPtr->getData(j) == 0 )
            {
                currentCategoryDeltaInGridPtr->setData(j,-1);
                currentCategoryDeltaOutGridPtr->setData(j,-1);
            }

            else
            {
                currentCategoryDeltaInGridPtr->setData(j, currentCategoryDeltaInGridPtr->getData(j)/currentCategoryHitGridPtr->getData(j) );
                currentCategoryDeltaOutGridPtr->setData(j, currentCategoryDeltaOutGridPtr->getData(j)/currentCategoryHitGridPtr->getData(j) );
            }
        }
    }

    // Calculate min and max in order to view the grid as a colored map later
    outputHitGrid->calculateMinMax();
    outputDeltaInGrid->calculateMinMax();
    outputDeltaOutGrid->calculateMinMax();

    for ( int i = 0 ; i < nCategories ; i++ )
    {
        categoryHitGridsList[i]->calculateMinMax();
        categoryDeltaInGridsList[i]->calculateMinMax();
        categoryDeltaOutGridsList[i]->calculateMinMax();
    }
}

CT_RegularGridInt* computeGridTools::computeTheoriticalGrid(qint64 id, Result *result, const CT_Point &bot, const CT_Point &top, float res, const CT_Scanner &scanner)
{
    // Creating result grid
    CT_RegularGridInt* theoriticalGrid = new CT_RegularGridInt(NULL,id, result, top, bot, res);

    // Creating visitors
    VisitorRaytracingIncrement* visitorIncrement = new VisitorRaytracingIncrement();

    // Creating two buffering CT_Point
    CT_Point tmp1, tmp2;

    // Creating the lists of grids/visitor
    QList< CT_AbstractRegularGridDataInterface* > gridsList;
    QList< AbstractVisitorRaytracing* > visitorsList;
    gridsList << theoriticalGrid;
    visitorsList << visitorIncrement;

    // Creates the ray traversal algorithm
    AlgorithmWoo algorithm;

    int nHorizontalRays = scanner.getNHRays();
    int nVerticalRays = scanner.getNVRays();
    int totalRays = nVerticalRays * nHorizontalRays;

    // For all theoritical rays of the scanner
    for ( int i = 0 ; i < nHorizontalRays ; i++ )
    {
        for ( int j = 0 ; j < nVerticalRays ; j++ )
        {
            // Get the next ray
            CT_Ray* currentRay = new CT_Ray( scanner.ray(i,j) );

            // Run the algorithm only if the ray intersects the grid
            if ( currentRay->intersect(theoriticalGrid, tmp1, tmp2) )
            {
                // Initialize the algorithm with current parameters
                algorithm.reset( *currentRay, theoriticalGrid );

                // Run the algorithm on the grids via the visitors
                algorithm.run( gridsList, visitorsList );
            }

            // Deleting the ray
            delete( currentRay );
        }
    }

    // Don't forget to calculate min and max in order to visualize it as a colored map
    theoriticalGrid->calculateMinMax();

    return theoriticalGrid;
}

void computeGridTools::computeTheoriticalGridAndDistances(const CT_Point &bot,
                                                          const CT_Point &top,
                                                          float res,
                                                          const CT_Scanner &scanner,
                                                          qint64 idTheoritical,
                                                          Result *resultTheoritical,
                                                          CT_RegularGridInt *&outputTheoriticalGrid,
                                                          qint64 idDeltaTheoritical,
                                                          Result *resultDeltaTheoritical,
                                                          CT_RegularGridDouble *&outputDeltaTheoriticalGrid)
{
    // Make sure parameters are good
    assert ( outputTheoriticalGrid == NULL && outputDeltaTheoriticalGrid == NULL );

    // Allocate results
    outputTheoriticalGrid = new CT_RegularGridInt(NULL, idTheoritical, resultTheoritical, top, bot, res, 0);
    outputDeltaTheoriticalGrid = new CT_RegularGridDouble (NULL,idDeltaTheoritical, resultDeltaTheoritical, top, bot, res, 0 );

    // Creating visitors
    VisitorRaytracingIncrement* visitorIncrement = new VisitorRaytracingIncrement();
    VisitorRaytracingAddDistance* visitorDistances = new VisitorRaytracingAddDistance();

    // Creating two buffering CT_Point
    CT_Point tmp1, tmp2;

    // Creating the lists of grids/visitor
    QList< CT_AbstractRegularGridDataInterface* > gridsList;
    QList< AbstractVisitorRaytracing* > visitorsList;
    gridsList << outputTheoriticalGrid << outputDeltaTheoriticalGrid;
    visitorsList << visitorIncrement << visitorDistances;

    // Creates the ray traversal algorithm
    AlgorithmWoo algorithm;

    int nHorizontalRays = scanner.getNHRays();
    int nVerticalRays = scanner.getNVRays();
    int totalRays = nVerticalRays * nHorizontalRays;

    // For all theoritical rays of the scanner
    for ( int i = 0 ; i < nHorizontalRays ; i++ )
    {
        for ( int j = 0 ; j < nVerticalRays ; j++ )
        {
            // Get the next ray
            CT_Ray* currentRay = new CT_Ray( scanner.ray(i,j) );

            // Run the algorithm only if the ray intersects the grid
            if ( currentRay->intersect(outputTheoriticalGrid, tmp1, tmp2) )
            {
                // Initialize the algorithm with current parameters
                algorithm.reset( *currentRay, outputTheoriticalGrid );

                // Setting the ray of the visitor
                visitorDistances->setRay( currentRay );

                // Run the algorithm on the grids via the visitors
                algorithm.run( gridsList, visitorsList );
            }

            // Deleting the ray
            delete( currentRay );
        }
    }

    // To get the mean distance we have to divide in each voxel the sum of distances by the number of hits
    for ( unsigned int i = 0 ; i < outputTheoriticalGrid->nVoxels() ; i++ )
    {
        if ( outputTheoriticalGrid->getData(i) == 0 )
        {
            outputDeltaTheoriticalGrid->setData(i,-1);
        }

        else
        {
            outputDeltaTheoriticalGrid->setData(i, outputDeltaTheoriticalGrid->getData(i)/outputTheoriticalGrid->getData(i) );
        }
    }

    // Don't forget to calculate min and max in order to visualize it as a colored map
    outputTheoriticalGrid->calculateMinMax();
    outputDeltaTheoriticalGrid->calculateMinMax();
}

CT_RegularGridInt* computeGridTools::computeBeforeGrid(qint64 id, Result *result, const CT_Point &bot, const CT_Point &top, float res, const CT_AbstractPointCloud *inputCloud, const CT_Scanner &scanner)
{
    // Creating result grid
    CT_RegularGridInt* beforeGrid = new CT_RegularGridInt(NULL,id, result, top, bot, res);

    // Creating visitors
    VisitorRaytracingIncrementButFirst* visitorIncrementButFirst = new VisitorRaytracingIncrementButFirst();

    // Creating a buffering CT_Point
    CT_Point direction;

    // Creating the lists of grids/visitor
    QList< CT_AbstractRegularGridDataInterface* > gridsList;
    QList< AbstractVisitorRaytracing* > visitorsList;
    gridsList << beforeGrid;
    visitorsList << visitorIncrementButFirst;

    // Creates the ray traversal algorithm
    AlgorithmWoo algorithm;

    int cloudSize = inputCloud->cloudSize();

    // For each point of the scene
    for ( int i = 0 ; i < cloudSize ; i++ )
    {
        // Create a ray which origin is the point of the scene and which direction is "the direction"from the scanner to the point"
        // Getting the direction
        direction.x = (*inputCloud)[i].x - scanner.getPosition().x();
        direction.y = (*inputCloud)[i].y - scanner.getPosition().y();
        direction.z = (*inputCloud)[i].z - scanner.getPosition().z();

        if ( normCtPoint(direction) != 0 )
        {
            // Creating the ray
            CT_Ray* currentRay = new CT_Ray(NULL,0, NULL, (*inputCloud)[i], direction );

            // Initializing the algorithm with te right parameters
            algorithm.reset( *currentRay, beforeGrid );

            // Running the algorithm over the grid
            algorithm.run( gridsList, visitorsList );
			
			// Reset the visitor
			visitorIncrementButFirst->reset();

            // Deleting the ray
            delete( currentRay );
        }
    }

    // Don't forget to calculate min and max in order to visualize it as a colored map
    beforeGrid->calculateMinMax();

    return beforeGrid;
}

void computeGridTools::computeBeforeGridAndDistances(const CT_Point &bot,
                                                     const CT_Point &top,
                                                     float res,
                                                     const CT_AbstractPointCloud *inputCloud,
                                                     const CT_Scanner &scanner,
                                                     qint64 idBefore,
                                                     Result *resultBefore,
                                                     CT_RegularGridInt *&outputBeforeGrid,
                                                     qint64 idDeltaBefore,
                                                     Result *resultDeltaBefore, CT_RegularGridDouble *&outputDeltaBeforeGrid)
{
    // Make sure parameters are good
    assert ( outputBeforeGrid == NULL && outputDeltaBeforeGrid == NULL );

    // Allocate results
    outputBeforeGrid = new CT_RegularGridInt(NULL, idBefore, resultBefore, top, bot, res, 0);
    outputDeltaBeforeGrid = new CT_RegularGridDouble (NULL, idDeltaBefore, resultDeltaBefore, top, bot, res, 0 );

    // Creating visitors
    VisitorRaytracingIncrementButFirst* visitorIncrementButFirst = new VisitorRaytracingIncrementButFirst();
    VisitorRaytracingAddDistanceButFirst* visitorDistancesButFirst = new VisitorRaytracingAddDistanceButFirst();

    // Creating a buffering CT_Point
    CT_Point direction;

    // Creating the lists of grids/visitor
    QList< CT_AbstractRegularGridDataInterface* > gridsList;
    QList< AbstractVisitorRaytracing* > visitorsList;
    gridsList << outputBeforeGrid << outputDeltaBeforeGrid;
    visitorsList << visitorIncrementButFirst << visitorDistancesButFirst;

    // Creates the ray traversal algorithm
    AlgorithmWoo algorithm;

    int cloudSize = inputCloud->cloudSize();

    // For each point of the scene
    for ( int i = 0 ; i < cloudSize ; i++ )
    {
        // Create a ray which origin is the point of the scene and which direction is "the direction"from the scanner to the point"
        // Getting the direction
        direction.x = (*inputCloud)[i].x - scanner.getPosition().x();
        direction.y = (*inputCloud)[i].y - scanner.getPosition().y();
        direction.z = (*inputCloud)[i].z - scanner.getPosition().z();

        if ( normCtPoint(direction) != 0 )
        {
            // Creating the ray
            CT_Ray* currentRay = new CT_Ray(NULL, 0, NULL, (*inputCloud)[i], direction );

            // Initializing the algorithm with the right parameters
            algorithm.reset( *currentRay, outputBeforeGrid );

            // Setting the ray of the visitor
            visitorDistancesButFirst->setRay( currentRay );

            // Running the algorithm over the grid
            algorithm.run( gridsList, visitorsList );
			
			// Reset the visitor
			visitorIncrementButFirst->reset();
			visitorDistancesButFirst->reset();

            // Deleting the ray
            delete( currentRay );
        }
    }

    // To get the mean distance we have to divide in each voxel the sum of distances by the number of hits
    for ( unsigned int i = 0 ; i < outputBeforeGrid->nVoxels() ; i++ )
    {
        if ( outputBeforeGrid->getData(i) == 0 )
        {
            outputDeltaBeforeGrid->setData(i,-1);
        }

        else
        {
            outputDeltaBeforeGrid->setData(i, outputDeltaBeforeGrid->getData(i)/outputBeforeGrid->getData(i) );
        }
    }

    // Don't forget to calculate min and max in order to visualize it as a colored map
    outputBeforeGrid->calculateMinMax();
    outputDeltaBeforeGrid->calculateMinMax();
}

CT_RegularGridDouble* computeGridTools::computeDensityGrid(qint64 id, Result *result, CT_RegularGridInt *hitsGrid, CT_RegularGridInt *theoriticalGrid, CT_RegularGridInt *beforeGrid, int effectiveRayThresh)
{
    // Checks if all grids have the same dimensions
    assert( hitsGrid->getDim().x == theoriticalGrid->getDim().x && hitsGrid->getDim().x == beforeGrid->getDim().x );
    assert( hitsGrid->getDim().y == theoriticalGrid->getDim().y && hitsGrid->getDim().y == beforeGrid->getDim().y );
    assert( hitsGrid->getDim().z == theoriticalGrid->getDim().z && hitsGrid->getDim().z == beforeGrid->getDim().z );

    // Creating result grid
    CT_RegularGridDouble* densityGrid = new CT_RegularGridDouble(NULL, id, result, hitsGrid->getTop(), hitsGrid->getBot(), hitsGrid->getRes() );

    unsigned int nbVoxels = densityGrid->nVoxels();

    // For each voxel
    for ( unsigned int i = 0 ; i < nbVoxels ; i++ )
    {
        // Compute the density index
        // If there is not enough information
        if ( theoriticalGrid->getData(i) - beforeGrid->getData(i) < effectiveRayThresh )
        {
            densityGrid->setData(i, -3);
        }

        // If there is an error (nb > nt)
        else if ( theoriticalGrid->getData(i) - beforeGrid->getData(i) < 0 )
        {
            densityGrid->setData(i, -2);
        }

        // Avoid division by 0
        else if ( theoriticalGrid->getData(i) - beforeGrid->getData(i) == 0 )
        {
            densityGrid->setData(i, -1);
        }

        // Normal case
        else
        {
            densityGrid->setData(i, (double)(hitsGrid->getData(i)) / (double)(theoriticalGrid->getData(i) - beforeGrid->getData(i) ) );
        }
    }

    // Don't forget to calculate min and max in order to visualize it as a colored map
    densityGrid->calculateMinMax();

    return densityGrid;
}

CT_RegularGridInt* computeGridTools::computeBestPointOfViewGrid(qint64 id,
                                                                Result *result,
                                                                const CT_Point &bot,
                                                                const CT_Point &top,
                                                                float res, QList<CT_RegularGridInt *> &theoriticalGridsList,
                                                                QList<CT_RegularGridInt *> &beforeGridsList,
                                                                QList<CT_RegularGridInt *> &HitsGridsList)
{
    // Make sure there is as many theoritical grid as before grids
    assert( theoriticalGridsList.size() == beforeGridsList.size() );

    // Creating the result
    CT_RegularGridInt* bestPointOfViewGrid = new CT_RegularGridInt(NULL, id, result, top, bot, res );

    // The best point of view is defined as the one that maximises the following index : Nt-Nb
    int currentIndex;
    int currentIndexMax;

    // Declaring an integer to find the maximum of Nt-Nb
    int max = INT_MIN;

    // Two integers storing the Nb and Nt for each voxel
    int currentNt;
    int currentNb;

    // For each voxel
    unsigned int nbVoxels = bestPointOfViewGrid->nVoxels();
    unsigned int listsSize = theoriticalGridsList.size();
    for ( unsigned int i = 0 ; i < nbVoxels ; i++ )
    {
        max = INT_MIN;
        currentIndexMax = 0;

        // For each couple of (theoritical, before) grids
        for ( unsigned int j = 0 ; j < listsSize ; j++ )
        {
            // If there is no hits in the voxel, then current index = -3
            if ( HitsGridsList[j]->getData(i) <= 0 )
            {
                currentIndex = -3;
            }

            else
            {
                // On veut maximiser Nt-Nb
                // Calculate the Nt-Nb of this voxel for each couple
                currentNt = theoriticalGridsList[j]->getData(i);
                currentNb = beforeGridsList[j]->getData(i);

                if ( currentNb == 0 )
                    currentIndex = -2;

                else
                    currentIndex = currentNt - currentNb;
            }

            if ( currentIndex > max )
            {
                max = currentIndex;
                currentIndexMax = j;
            }
        }

        // Set to -1 if an error occured
        // Set to -2 if nb = O
        // Set to -3 if no hits
        if ( max < 0 )
        {
            if ( max == -3 )
                bestPointOfViewGrid->setData(i,-3);

            else if ( max == -2 )
                bestPointOfViewGrid->setData(i,-2);

            else
                bestPointOfViewGrid->setData(i,-1);
        }


        // The best point of view index else
        else
            bestPointOfViewGrid->setData(i,currentIndexMax);
    }

    bestPointOfViewGrid->calculateMinMax();

    return bestPointOfViewGrid;
}

int computeGridTools::getCategory(float intensity, int nCategories, const vector<int>& benchmarks)
{
    unsigned int category = 0;

    while( category < nCategories-1 && intensity >= benchmarks[category] )
    {
        category++;
    }

    return category;
}

void computeGridTools::saveDensityOnlyResult(CT_RegularGridDouble *densityGrid,
                                             CT_RegularGridInt *hitGrid,
                                             CT_RegularGridInt *theoriticalGrid,
                                             CT_RegularGridInt *beforeGrid,
                                             QList<CT_RegularGridInt *> categoryHitGridList,
                                             int nCategories,
                                             const vector<int>& categoriesBenchmarks,
                                             QString filePath,
                                             QString fileName )
{
    // Make sure parameters are valid
    assert ( densityGrid->nVoxels() == hitGrid->nVoxels() && densityGrid->nVoxels() == theoriticalGrid->nVoxels() && densityGrid->nVoxels() == beforeGrid->nVoxels() );

    // Declaring and opening the output file
    QFile outputFile(filePath+fileName);
    if ( !outputFile.open( QIODevice::WriteOnly ) )
    {
        qDebug() << "Could not open the output file " << filePath+fileName;
        qDebug() << "Leaving Computree";
        exit(1);
    }

    // Header will contain the bounding box of the grids on the first line, the number of voxels on the second line, the number of categories on the third and the categories benchmarks on the fourth
    QTextStream stream( &outputFile );
	
	// Write header comments
	stream << "# LVox result file from the Computree plugin" << endl;
	stream << "# Result from the \"" << filePath+fileName << "\" input file" << endl;
	stream << "# Each line starting with the '#' character is considered as a comment" << endl;
	stream << "# This file is made of a header - 4 lines - containing : " << endl;
	stream << "# \t1) The bounding box of the process" << endl;
	stream << "# \t2) The size of a voxel and the number of voxels" << endl;
	stream << "# \t3) the number of categories" << endl;
	stream << "# \t4) The thresholds between categories" << endl;
	stream << "# Each of the following lines contains \'density Ni Nt Nb [Ni_Category_j]\'" << endl;
	
    // Write header to the file (bounding box)
    float xMin = densityGrid->getBot().x;
    float yMin = densityGrid->getBot().y;
    float zMin = densityGrid->getBot().z;
    float xMax = densityGrid->getTop().x;
    float yMax = densityGrid->getTop().y;
    float zMax = densityGrid->getTop().z;

    stream << xMin << " " << yMin << " " << zMin << " " << xMax << " " << yMax << " " << zMax << "\n";
    stream << densityGrid->getRes() << " " << densityGrid->nVoxels() << "\n";
    stream << nCategories << "\n";

    if ( nCategories > 1 )
    {
        stream << categoriesBenchmarks[0];
        for ( int i = 1 ; i < nCategories-1 ; i++ )
        {
            stream << " " << categoriesBenchmarks[i];
        }
        stream << "\n";
    }

    // If there is only one category, the benchmark is set to -1
    else
    {
        stream << "-1\n";
    }

    // For each voxel
    unsigned int nbVoxels = densityGrid->nVoxels();
    for ( unsigned int i = 0 ; i < nbVoxels ; i++ )
    {
        // Write "density Ni Nt Nb"
        stream << densityGrid->getData(i) << " " << hitGrid->getData(i) << " " << theoriticalGrid->getData(i) << " " << beforeGrid->getData(i);

        if ( nCategories > 1 )
        {
            // And the Ni for each categories
            for ( int j = 0 ; j < nCategories ; j++ )
            {
                stream << " " << categoryHitGridList[j]->getData(i) ;
            }

            stream << "\n";
        }

        else
        {
            stream << "\n";
        }

    }

    outputFile.close();
}

void computeGridTools::saveDensityOnlyResult(const QList<CT_RegularGridDouble *> &densityGridList,
                                             const QList<CT_RegularGridInt *> &hitGridList,
                                             const QList<CT_RegularGridInt *> &theoriticalGrid,
                                             const QList<CT_RegularGridInt *> &beforeGrid,
                                             const QList< QList<CT_RegularGridInt *> > &categoryHitGridListList,
                                             const CT_RegularGridInt *bestPointOfViewGrid,
                                             int nCategories, const vector<int> &categoriesBenchmarks,
                                             QString filePath,
                                             QString fileName)
{
    // Make sure parameters are valid
    assert( densityGridList.size() == hitGridList.size() && densityGridList.size() == theoriticalGrid.size() && densityGridList.size() == beforeGrid.size() );

    if ( nCategories > 1 )
    {
        // For each scanner, write the result in a separated file
        unsigned int nScanners = densityGridList.size();
        for ( unsigned int i = 0 ; i < nScanners ; i++ )
        {
            QList< CT_RegularGridInt* > categoryHitListFromCurrentScanner;
            for ( int j = 0 ; j < nCategories ; j++ )
            {
                categoryHitListFromCurrentScanner.push_back( categoryHitGridListList[j][i] );
            }

            computeGridTools::saveDensityOnlyResult( densityGridList[i], hitGridList[i], theoriticalGrid[i], beforeGrid[i], categoryHitListFromCurrentScanner, nCategories, categoriesBenchmarks, filePath, fileName + QString("_scanner_") + QString().number(i) + QString(".ldf") );
        }
    }

    else
    {
        // For each scanner, write the result in a separated file
        unsigned int nScanners = densityGridList.size();
        for ( unsigned int i = 0 ; i < nScanners ; i++ )
        {
            computeGridTools::saveDensityOnlyResult( densityGridList[i], hitGridList[i], theoriticalGrid[i], beforeGrid[i], QList< CT_RegularGridInt* > (), nCategories, categoriesBenchmarks, filePath, fileName + QString("_scanner_") + QString().number(i) + QString(".ldf") );
        }
    }

    // Now write the merged results as :
    // "densityOfBestPointOfView NiOfBestPointOfView NtOfBestPointOfView NbOfBestPointOfView [Ni_ith_CategoryOfBestPointOfView] bestPointOfView"
    // Declaring and opening the output file
    QFile outputFile( filePath+fileName+QString(".mdf") );
    if ( !outputFile.open( QIODevice::WriteOnly ) )
    {
        qDebug() << "Could not open the output file " << filePath+fileName;
        qDebug() << "Leaving Computree";
        exit(1);
    }

    QTextStream stream( &outputFile );
	
	// Writing header comments
	stream << "# LVox result file from the Computree plugin" << endl;
	stream << "# Result from the \"" << filePath+fileName << "\" input file" << endl;
	stream << "# Each line starting with the '#' character is considered as a comment" << endl;
	stream << "# This file is made of a header - 4 lines - containing : " << endl;
	stream << "# \t1) The bounding box of the process" << endl;
	stream << "# \t2) The size of a voxel and the number of voxels" << endl;
	stream << "# \t3) the number of categories" << endl;
	stream << "# \t4) The thresholds between categories" << endl;
	stream << "# Each of the following lines contains \'density Ni Nt Nb [Ni_Category_j]\' from the best point of view and \'best_point_of_view\'" << endl;
	
    // Header will contain the bounding box of the grids on the first line, the number of voxels on the second line, the number of categories on the third and the categories benchmarks on the fourth
    // Write header to the file (bounding box)
    float xMin = densityGridList[0]->getBot().x;
    float yMin = densityGridList[0]->getBot().y;
    float zMin = densityGridList[0]->getBot().z;
    float xMax = densityGridList[0]->getTop().x;
    float yMax = densityGridList[0]->getTop().y;
    float zMax = densityGridList[0]->getTop().z;
    stream << xMin << " " << yMin << " " << zMin << " " << xMax << " " << yMax << " " << zMax << "\n";
    stream << densityGridList[0]->getRes() << " " << densityGridList[0]->nVoxels() << "\n";
    stream << nCategories << "\n";

    if ( nCategories > 1 )
    {
        stream << categoriesBenchmarks[0];
        for ( int i = 1 ; i < nCategories-1 ; i++ )
        {
            stream << " " << categoriesBenchmarks[i];
        }
        stream << "\n";
    }

    // If there is only one category, the benchmark is set to -1
    else
    {
        stream << "-1\n";
    }

    // Writing merged file
    unsigned int nVoxels = densityGridList[0][0].nVoxels();
    int bestPointOfView;
    // For each voxel
    for ( unsigned int i = 0 ; i < nVoxels ; i++ )
    {
        // Get the best point of view for the current voxel
        bestPointOfView = bestPointOfViewGrid->getData(i);

        // If the best point of view was computed without error for this voxel
        if ( bestPointOfView >= 0 )
        {
            // Write "Density Ni Nt Nb"
            stream << densityGridList[ bestPointOfView ]->getData(i) << " " << hitGridList[ bestPointOfView ]->getData(i) << " " << theoriticalGrid[ bestPointOfView ]->getData(i) << " " << beforeGrid[bestPointOfView]->getData(i);

            if ( nCategories > 1 )
            {
                // Write categories Ni
                for ( unsigned int j = 0 ; j < nCategories ; j++ )
                {
                    stream << " " << categoryHitGridListList[j][bestPointOfView]->getData(i);
                }
            }

            // Write the best point of view
            stream << " " << bestPointOfView << "\n";
        }

        else
            stream << "no_data\n";
    }

    outputFile.close();
}

void computeGridTools::saveDensityAndDistancesResult (  CT_RegularGridDouble* densityGrid,
                                                        CT_RegularGridInt* hitGrid,
                                                        CT_RegularGridInt* theoriticalGrid,
                                                        CT_RegularGridInt* beforeGrid,
                                                        CT_RegularGridDouble* deltaTheoriticalGrid,
                                                        CT_RegularGridDouble* deltaBeforeGrid,
                                                        CT_RegularGridDouble* deltaInGrid,
                                                        CT_RegularGridDouble* deltaOutGrid,
                                                        QList<CT_RegularGridInt *> categoryHitGridList,
                                                        QList<CT_RegularGridDouble *> categoryDeltaInGridList,
                                                        QList<CT_RegularGridDouble *> categoryDeltaOutGridList,
                                                        int nCategories,
                                                        const vector<int>& categoriesBenchmarks,
                                                        QString filePath,
                                                        QString fileName,
                                                        QString extension )
{
    // Make sure parameters are valid
    assert ( densityGrid->nVoxels() == hitGrid->nVoxels() && densityGrid->nVoxels() == theoriticalGrid->nVoxels() && densityGrid->nVoxels() == beforeGrid->nVoxels() );
    assert ( densityGrid->nVoxels() == deltaInGrid->nVoxels() && densityGrid->nVoxels() == deltaOutGrid->nVoxels() );
    if ( nCategories > 1 )
		assert ( categoryHitGridList.size() == nCategories && categoryDeltaInGridList.size() == nCategories && categoryDeltaOutGridList.size() == nCategories );

    // Declaring and opening the output file
    QString filePathAndName;

    if ( extension != "" )
        filePathAndName = filePath+QString("/")+fileName+extension;

    else
        filePathAndName = filePath+QString("/")+fileName+QString(".mox");

    QFile outputFile( filePathAndName );
    if ( !outputFile.open( QIODevice::WriteOnly ) )
    {
        qDebug() << "Could not open the output file " << filePath+fileName;
        qDebug() << "Leaving Computree";
        exit(1);
    }

    // Header will contain the bounding box of the grids on the first line, the number of voxels on the second line, the number of categories on the third and the categories benchmarks on the fourth
    QTextStream stream( &outputFile );
	
	
	// Writing header comments
	stream << "# LVox result file from the Computree plugin" << endl;
	stream << "# Result from the \"" << filePath+fileName << "\" input file" << endl;
	stream << "# Each line starting with the '#' character is considered as a comment" << endl;
	stream << "# This file is made of a header - 5 lines - containing : " << endl;
	stream << "# \t1) The bounding box of the process" << endl;
	stream << "# \t2) The size of a voxel and the number of voxels" << endl;
	stream << "# \t3) the number of categories" << endl;
	stream << "# \t4) The thresholds between categories" << endl;
	stream << "# \t5) The key word \"PLUGIN\"" << endl;
	stream << "# Each of the following lines contains \'density Ni Nt Nb deltaTheo deltaBefore deltaIn deltaOut [Ni_Category_j deltaIn_Category_j deltaOut_Category_j]\'" << endl;
	
    // Write header to the file (bounding box)
    float xMin = densityGrid->getBot().x;
    float yMin = densityGrid->getBot().y;
    float zMin = densityGrid->getBot().z;
    float xMax = densityGrid->getTop().x;
    float yMax = densityGrid->getTop().y;
    float zMax = densityGrid->getTop().z;

    stream << xMin << " " << yMin << " " << zMin << " " << xMax << " " << yMax << " " << zMax << "\n";
    stream << densityGrid->getRes() << " " << densityGrid->nVoxels() << "\n";
    stream << nCategories << "\n";

    if ( nCategories > 1 )
    {
        stream << categoriesBenchmarks[0];
        for ( int i = 1 ; i < nCategories-1 ; i++ )
        {
            stream << " " << categoriesBenchmarks[i];
        }
        stream << "\n";
    }

    // If there is only one category, the benchmark is set to -1
    else
    {
        stream << "-1\n";
    }
	
	stream << "PLUGIN" << endl;

    // For each voxel
    unsigned int nbVoxels = densityGrid->nVoxels();
    for ( unsigned int i = 0 ; i < nbVoxels ; i++ )
    {
        // Write "density Ni Nt Nb deltaTheoritical deltaBefore deltaIn deltaOut Ni_ith_category deltaIn_ith_category deltaOut_ith_category"
        stream << densityGrid->getData(i) << " " << hitGrid->getData(i) << " " << theoriticalGrid->getData(i) << " " << beforeGrid->getData(i) << " " <<
                  deltaTheoriticalGrid->getData(i) << " " << deltaBeforeGrid->getData(i) << " " << deltaInGrid->getData(i) << " " << deltaOutGrid->getData(i);

        if ( nCategories > 1 )
        {
            // And the Ni for each categories
            for ( int j = 0 ; j < nCategories ; j++ )
            {
                stream << " " << categoryHitGridList[j]->getData(i) << " " << categoryDeltaInGridList[j]->getData(i) << " " << categoryDeltaOutGridList[j]->getData(i) ;
            }

            stream << "\n";
        }

        else
        {
            stream << "\n";
        }

    }

    outputFile.close();
}

void computeGridTools::saveDensityAndDistancesResult (   const QList< CT_RegularGridDouble* > & densityGridList,
                                                         const QList< CT_RegularGridInt* >& hitGridList,
                                                         const QList< CT_RegularGridInt* >& theoriticalGridList,
                                                         const QList< CT_RegularGridInt* >& beforeGridList,
                                                         const QList< CT_RegularGridDouble* >& deltaTheoriticalGridList,
                                                         const QList< CT_RegularGridDouble* >& deltaBeforeGridList,
                                                         const QList< CT_RegularGridDouble* >& deltaInGridList,
                                                         const QList< CT_RegularGridDouble* >& deltaOutGridList,
                                                         const QList< QList< CT_RegularGridInt* > >& categoryHitGridListList,
                                                         const QList< QList< CT_RegularGridDouble* > >& categoryDeltaInGridListList,
                                                         const QList< QList< CT_RegularGridDouble* > >& categoryDeltaOutGridListList,
                                                         const CT_RegularGridInt* bestPointOfViewGrid,
                                                         int nCategories,
                                                         const vector<int>& categoriesBenchmarks,
                                                         QString filePath,
                                                         QString fileName )
{
    // Make sure parameters are valid
    assert( densityGridList.size() == hitGridList.size() && densityGridList.size() == theoriticalGridList.size() && densityGridList.size() == beforeGridList.size() );
    assert( densityGridList.size() == deltaTheoriticalGridList.size() && densityGridList.size() == deltaBeforeGridList.size() && densityGridList.size() == deltaInGridList.size() && densityGridList.size() == deltaOutGridList.size() );
	if ( nCategories > 1 )
		assert( categoryHitGridListList.size() == nCategories && categoryDeltaOutGridListList.size() == nCategories && categoryDeltaInGridListList.size() == nCategories);

    // For each scanner, write the result in a separated file
    unsigned int nScanners = densityGridList.size();
    for ( unsigned int i = 0 ; i < nScanners ; i++ )
    {
        QList< CT_RegularGridInt* > categoryHitListFromCurrentScanner;
        QList< CT_RegularGridDouble* > categoryDeltaInListFromCurrentScanner;
        QList< CT_RegularGridDouble* > categoryDeltaOutListFromCurrentScanner;
        if ( nCategories > 1 )
		{
			for ( int j = 0 ; j < nCategories ; j++ )
			{
				categoryHitListFromCurrentScanner.push_back( categoryHitGridListList[j][i] );
				categoryDeltaInListFromCurrentScanner.push_back( categoryDeltaInGridListList[j][i] );
				categoryDeltaOutListFromCurrentScanner.push_back( categoryDeltaInGridListList[j][i] );
			}
		}

        computeGridTools::saveDensityAndDistancesResult( densityGridList[i], hitGridList[i], theoriticalGridList[i], beforeGridList[i], deltaTheoriticalGridList[i], deltaBeforeGridList[i], deltaInGridList[i], deltaOutGridList[i], categoryHitListFromCurrentScanner, categoryDeltaInListFromCurrentScanner, categoryDeltaOutListFromCurrentScanner, nCategories, categoriesBenchmarks, filePath, fileName + QString("_scanner_") + QString().number(i), QString(".vox") );
    }

    // Now write the merged results as :
    // "densityOfBestPointOfView NiOfBestPointOfView NtOfBestPointOfView NbOfBestPointOfView deltaTheoritical deltaBefore deltaIn deltaOut [Ni_ith_CategoryOfBestPointOfView deltaIn_ith_category deltaOut_ith_category] bestPointOfView"
    // Declaring and opening the output file
    QFile outputFile( filePath+QString("/")+fileName+QString(".mox") );

    if ( !outputFile.open( QIODevice::WriteOnly ) )
    {
        qDebug() << "Could not open the output file " << filePath+fileName;
        qDebug() << "Leaving Computree";
        exit(1);
    }

    QTextStream stream( &outputFile );

    // Writing merged file
	stream << "# LVox result file from the Computree plugin" << endl;
	stream << "# Result from the \"" << filePath+fileName << "\" input file" << endl;
	stream << "# Each line starting with the '#' character is considered as a comment" << endl;
	stream << "# This file is made of a header - 5 lines - containing : " << endl;
	stream << "# \t1) The bounding box of the process" << endl;
	stream << "# \t2) The size of a voxel and the number of voxels" << endl;
	stream << "# \t3) the number of categories" << endl;
	stream << "# \t4) The thresholds between categories" << endl;
	stream << "# \t5) The key word \"PLUGIN\"" << endl;
	stream << "# Each of the following lines contains \'density Ni Nt Nb deltaTheo deltaBefore deltaIn deltaOut [Ni_Category_j deltaIn_Category_j deltaOut_Category_j]\' from the best point of view and \'best_point_of_view\'" << endl;
	
    // Header will contain the bounding box of the grids on the first line, the number of voxels on the second line, the number of categories on the third and the categories benchmarks on the fourth
    // Write header to the file (bounding box)
    float xMin = densityGridList[0]->getBot().x;
    float yMin = densityGridList[0]->getBot().y;
    float zMin = densityGridList[0]->getBot().z;
    float xMax = densityGridList[0]->getTop().x;
    float yMax = densityGridList[0]->getTop().y;
    float zMax = densityGridList[0]->getTop().z;

    stream << xMin << " " << yMin << " " << zMin << " " << xMax << " " << yMax << " " << zMax << "\n";
    stream << densityGridList[0]->getRes() << " " << densityGridList[0]->nVoxels() << "\n";
    stream << nCategories << "\n";

    if ( nCategories > 1 )
    {
        stream << categoriesBenchmarks[0];
        for ( int i = 1 ; i < nCategories-1 ; i++ )
        {
            stream << " " << categoriesBenchmarks[i];
        }
        stream << "\n";
    }

    // If there is only one category, the benchmark is set to -1
    else
    {
        stream << "-1\n";
    }
	
	stream << "PLUGIN" << endl;
	
    unsigned int nVoxels = densityGridList[0][0].nVoxels();
    int bestPointOfView;
    // For each voxel
    for ( unsigned int i = 0 ; i < nVoxels ; i++ )
    {
        // Get the best point of view for the current voxel
        bestPointOfView = bestPointOfViewGrid->getData(i);

        // If the best point of view was computed without error for this voxel
        if ( bestPointOfView >= 0 )
        {
            // Write "Density Ni Nt Nb"
            stream << densityGridList[ bestPointOfView ]->getData(i) << " " << hitGridList[ bestPointOfView ]->getData(i) << " " << theoriticalGridList[ bestPointOfView ]->getData(i) << " " << beforeGridList[bestPointOfView]->getData(i) << " " << deltaTheoriticalGridList[ bestPointOfView ]->getData(i) << " " << deltaBeforeGridList[ bestPointOfView ]->getData(i) << " " << deltaInGridList[ bestPointOfView ]->getData(i) << " " << deltaOutGridList[ bestPointOfView ]->getData(i);

            if ( nCategories > 1 )
            {
                // Write categories Ni
                for ( unsigned int j = 0 ; j < nCategories ; j++ )
                {
                    stream << " " << categoryHitGridListList[j][bestPointOfView]->getData(i) << " " << categoryDeltaInGridListList[j][bestPointOfView]->getData(i) << " " << categoryDeltaOutGridListList[j][bestPointOfView]->getData(i);
                }
            }

            // Write the best point of view
            stream << " " << bestPointOfView << "\n";
        }

        else
            stream << "no_data\n";
    }

    outputFile.close();
}
