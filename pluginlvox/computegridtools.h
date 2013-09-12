/****************************************************************************

 Copyright (C) 2012-2012 Université de Sherbrooke, Québec, CANADA
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

#ifndef COMPUTEGRIDTOOLS_H
#define COMPUTEGRIDTOOLS_H

#include "ct_itemdrawable/ct_regulargridint.h"      // Using integers regular grids
#include "ct_itemdrawable/ct_regulargriddouble.h"   // Using doubles regular grids

#include "ct_itemdrawable/ct_scene.h"               // Using scenes to compute grids
#include "ct_itemdrawable/ct_scanner.h"             // Using scanners to compute grids

#include "algorithmewoo.h"                          // Raytracing algorithm
#include "visitorraytracingadddistance.h"           // and its
#include "visitorraytracingincrement.h"             // visitors
#include "visitorraytracingincrementbutfirst.h"		// .
#include "visitorraytracingadddistancebutfirst.h"	// .

#include <vector>                                   // Using vectors

#include <QDebug>                                   // Printing information on console while in debug mode

using namespace std;                                // Facilitate the use (writing) of vectors

namespace computeGridTools
{
    /*!
    *  \brief Computes the number of categories
    *
    *
    *  \return Returns the number of categories
    */
    int getNCategories ( vector<int>& benchmarks );

    /*!
    *  \brief Computes the "hit" grid of a point cloud
    *
    *  Each voxel of the resulting grid stores the number of hits inside itself
    *
    *  \param id : ID of the item drawable
    *  \param result : result to attach to the grid
    *  \param bot : bottom left corner of the grid
    *  \param top : top left corner of the grid
    *  \param res : resolution(size of each voxel) of the grid
    *  \param inputCloud : point cloud to take into account while computing the grid
    *  \param intensityThresh : minimum or maximum intensity for a hit to contribute to the results
    *  \param greaterThan : if true, only consider intensity greater than the threshold, otherwise only consider intensity below that threshold
    *
    *  \return Returns the hit grid storing the number of points inside each voxel
    */
    CT_RegularGridInt* computeHitGrid( qint64 id,
                                       Result* result,
                                       const CT_Point& bot,
                                       const CT_Point& top,
                                       float res,
                                       const CT_AbstractPointCloud* inputCloud,
                                       double intensityThresh,
                                       bool greaterThan );

    // Usefull for further multi threading programming (couple with the computeHitGrid method)
    /*!
    *  \brief Computes the "hit" grid for each category of a given point cloud
    *
    *  Each voxel of a category grids store the number of hit belonging to this category in this voxel
    *
    *  \param bot : bottom left corner of the grid
    *  \param top : top left corner of the grid
    *  \param res : resolution(size of each voxel) of the grid
    *  \param inputCloud : point cloud to take into account while computing the grid
    *  \param intensityThresh : minimum intensity for a hit to contribute to the results
    *  \param greaterThan : if true, only consider intensity greater than the threshold, otherwise only consider intensity below that threshold
    *  \param categoryHitGridList : list of the category hit grids to calculate
    *  \param idList : list of index corresponding to the category hit grids
    *  \param greaterThan : if true, only consider intensity greater than the threshold, otherwise only consider intensity below that threshold
    *  \param nCategories : number of categories
    *  \param benchmarks : thresholds between categories
    *
    */
    void computeCategoriesHitGrids( const CT_Point& bot,
                                    const CT_Point& top,
                                    float res,
                                    const CT_AbstractPointCloud* inputCloud,
                                    double intensityThresh,
                                    bool greaterThan,
                                    QList< CT_RegularGridInt* >& categoryHitGridList,
                                    QList< qint64 > idList,
                                    QList< Result* > resultList,
                                    int nCategories,
                                    vector<int>& benchmarks );

    /*!
    *  \brief Computes the "hit" grid for each category of a given point cloud and the main hit grid
    *
    *  Each voxel of a category grids store the number of hit belonging to this category in this voxel, the main hit grid does not take into account the categories
    *
    *  \param id : ID of the main hit grid
    *  \param result : result to attach to the main grid
    *  \param bot : bottom left corner of the grid
    *  \param top : top left corner of the grid
    *  \param res : resolution(size of each voxel) of the grid
    *  \param inputCloud : point cloud to take into account while computing the grid
    *  \param intensityThresh : minimum intensity for a hit to contribute to the results
    *  \param greaterThan : if true, only consider intensity greater than the threshold, otherwise only consider intensity below that threshold
    *  \param categoryHitGridList : list of the category hit grids to calculate
    *  \param idList : list of index corresponding to the category hit grids
    *  \param resultList : list of the result associated to the category hit grids
    *  \param nCategories : number of categories
    *  \param benchmarks : thresholds between categories
    *
    *  \return Returns the main hit grid storing the number of points inside each voxel
    *
    */
    CT_RegularGridInt* computeAllHitGrids( qint64 id,
                                           Result* result,
                                           const CT_Point& bot,
                                           const CT_Point& top,
                                           float res,
                                           const CT_AbstractPointCloud* inputCloud,
                                           double intensityThresh,
                                           bool greaterThan,
                                           QList< CT_RegularGridInt* >& categoryHitGridList,
                                           QList< qint64 > idList,
                                           QList< Result* > resultList,
                                           int nCategories,
                                           vector<int>& benchmarks);

    /*!
    *  \brief Computes the "hit" grid of a point cloud and the grids of distance associated to it (deltaIn and deltaOut)
    *
    *  Each voxel of the "hit" grid stores the number of hits inside itself
    *  Each voxel of the "deltaIn" grid stores the average time spend by the rays in this voxel
    *  Each voxel of the "deltaOut" grid stores the average time spend by the rays in this voxel
    *  \warning The ID and the attached result of the different grid are not set in this method
    *
    *  \param bot : bottom left corner of the grid
    *  \param top : top left corner of the grid
    *  \param res : resolution(size of each voxel) of the grid
    *  \param inputCloud : point cloud to take into account while computing the grid
    *  \param scanner : scanner that launches ray through the grid
    *
    *  \param outputHitGrid : output hit grid
    *  \param outputDeltaInGrid : output deltaOut grid
    *  \param outputOutGrid : output deltaIn grid
    *
    */
    void computeHitGridAndDistances(const CT_Point &bot,
                                     const CT_Point& top,
                                     float res,
                                     const CT_AbstractPointCloud *inputCloud,
                                     const CT_Scanner *scanner,
                                     double intensityThresh,
                                     bool greaterThan,
                                     CT_RegularGridInt *outputHitGrid,
                                     CT_RegularGridDouble *outputDeltaInGrid,
                                     CT_RegularGridDouble *outputDeltaOutGrid );

    /*!
    *  \brief Computes the "hit" grid of a point cloud and the grids of distance associated to it (deltaIn and deltaOut) in a global manner and for each category
    *
    *  Each voxel of the "hit" grid stores the number of hits inside itself
    *  Each voxel of the "deltaIn" grid stores the average time spend by the rays in this voxel
    *  Each voxel of the "deltaOut" grid stores the average time spend by the rays in this voxel
    *  \warning The ID and the attached result of the different grid are not set in this method
    *
    *  \param bot : bottom left corner of the grid
    *  \param top : top left corner of the grid
    *  \param res : resolution(size of each voxel) of the grid
    *  \param inputCloud : point cloud to take into account while computing the grid
    *  \param scanner : scanner that launches ray through the grid
    *
    *  \param outputHitGrid : output hit grid
    *  \param outputDeltaInGrid : output deltaOut grid
    *  \param outputOutGrid : output deltaIn grid
    *
    *  \param categoryHitGridsList
    *
    */
    void computeHitGridAndDistancesAndCategories( const CT_Point &bot,
                                                  const CT_Point &top,
                                                  float res,
                                                  const CT_AbstractPointCloud *inputCloud,
                                                  const CT_Scanner* scanner,
                                                  double intensityThresh,
                                                  bool greaterThan,
                                                  CT_RegularGridInt *outputHitGrid,
                                                  CT_RegularGridDouble *outputDeltaInGrid,
                                                  CT_RegularGridDouble *outputDeltaOutGrid,
                                                  QList<CT_RegularGridInt *> &categoryHitGridsList,
                                                  QList<CT_RegularGridDouble *> &categoryDeltaInGridsList,
                                                  QList<CT_RegularGridDouble *> &categoryDeltaOutGridsList,
                                                  int nCategories,
                                                  const vector<int> &categoriesBenchmarks );

    /*!
    *  \brief Computes the "theoritical" grid of a point cloud
    *
    *  Each voxel of the resulting grid stores the number of theoritical rays from a given scanner intersecting this voxel
    *
    *  \param id : ID of the item drawable
    *  \param result : result to attach the grid
    *  \param bot : bottom left corner of the grid
    *  \param top : top left corner of the grid
    *  \param res : resolution(size of each voxel) of the grid
    *  \param scanner : scanner that launches ray through the grid
    *
    *  \return Returns the "theoritical" grid storing the number of theoritical rays inside each voxel
    */
    CT_RegularGridInt* computeTheoriticalGrid( qint64 id,
                                               Result* result,
                                               const CT_Point& bot,
                                               const CT_Point& top,
                                               float res,
                                               const CT_Scanner& scanner );

    /*!
    *  \brief Computes the "theoritical" grid of a point cloud and the distances associated (delta theoritical)
    *
    *  Each voxel of the resulting "hit" grid stores the number of theoritical rays from a given scanner intersecting this voxel
    *  Each voxel of the resulting "deltaTheoritical" grid stores the average time spent by the rays in this voxel
    *  \warning The ID and the attached result of the different grid are not set in this method
    *
    *  \param bot : bottom left corner of the grid
    *  \param top : top left corner of the grid
    *  \param res : resolution(size of each voxel) of the grid
    *  \param scanner : scanner that launches ray through the grid
    *
    *  \param idTheoritical : ID of the future theoritical grid
    *  \param resultTheoritical : result to attach to the future theoritical grid
    *  \param outputTheoriticalGrid : output theoritical grid
    *
    *  \param idDeltaTheoritical : ID of the future theoritical distances grid
    *  \param resultDeltaTheoritical : result to attach to the future theoritical distances grid
    *  \param outputDeltaTheoriticalGrid : output deltaTheoritical grid
    *
    */
    void computeTheoriticalGridAndDistances(const CT_Point& bot,
                                             const CT_Point& top,
                                             float res,
                                             CT_Scanner *scanner,
                                             CT_RegularGridInt *outputTheoriticalGrid,
                                             CT_RegularGridDouble *outputDeltaTheoriticalGrid );

    /*!
    *  \brief Computes the "before" grid of a point cloud
    *
    *  Each voxel of the resulting grid stores the number of rays that should have pierced the voxel if it had not been intercepted before
    *
    *  \param id : ID of the item drawable
    *  \param result : result to attach the grid
    *  \param bot : bottom left corner of the grid
    *  \param top : top left corner of the grid
    *  \param res : resolution(size of each voxel) of the grid
    *  \param inputCloud : point cloud to take into account while computing the grid
    *  \param scanner : scanner that launches ray through the grid
    *
    *  \return Returns the "before" grid storing the number of rays that should have pierced the voxel if it had not been intercepted before inside each voxel
    */
    CT_RegularGridInt* computeBeforeGrid( qint64 id,
                                          Result* result,
                                          const CT_Point& bot,
                                          const CT_Point& top,
                                          float res,
                                          const CT_AbstractPointCloud* inputCloud,
                                          const CT_Scanner& scanner );

    /*!
    *  \brief Computes the "before" grid of a point cloud and the associated distances (deltaBefore)
    *
    *  Each voxel of the output "before" grid stores the number of rays that should have pierced the voxel if it had not been intercepted before
    *  Each voxel of the output "deltaBefore" grid stores the average time spent by the rays in this voxel
    *
    *  \param bot : bottom left corner of the grid
    *  \param top : top left corner of the grid
    *  \param res : resolution(size of each voxel) of the grid
    *  \param inputCloud : point cloud to take into account while computing the grid
    *  \param scanner : scanner that launches ray through the grid
    *
    *  \param idBefore : ID of the future before grid
    *  \param resultBefore : result to attach to the future before grid
    *  \param outputBeforeGrid : output before grid
    *
    *  \param idDeltaBefore : ID of the future before distances grid
    *  \param resultDeltaBefore : result to attach to the future before distances grid
    *  \param outputDeltaBeforeGrid : output deltaBefore grid
    *
    */
    void computeBeforeGridAndDistances( const CT_Point &bot,
                                        const CT_Point &top,
                                        float res,
                                        const CT_AbstractPointCloud *inputCloud,
                                        CT_Scanner* scanner,
                                        CT_RegularGridInt *outputBeforeGrid,
                                        CT_RegularGridDouble *outputDeltaBeforeGrid);

    /*!
    *  \brief Computes density grid
    *
    *  Each voxel of the resulting grid stores the density index computed by LVox (ni/nt-nb)
    *
    *  \param id : ID of the item drawable
    *  \param result : result to attach the grid
    *  \param hitsGrid : "hits" grid previously computed
    *  \param theoriticalGrid : "theoritical" grid previously computed
    *  \param beforeGrid : "before" grid previously computed
    *  \param effectiveRayThresh : minimum number of effective rays (Nt-Nb) needed to compute a density index
    *  \param densityGrid : Returns the "density" grid storing the density index computed by LVox inside each voxel
    *
    *  \warning Values stored in each voxel may be negative :
    *           => -3 means there was not enough information to compute a density index (accordingly to the threshold on the number of effective rays)
    *           => -2 means there was an error (Nb > Nt)
    *           => -1 means Nt = Nb, and so the density index can not be calculated (division by 0)
    *
    */
    void computeDensityGrid(  CT_RegularGridDouble *densityGrid,
                              CT_RegularGridInt* hitsGrid,
                              CT_RegularGridInt* theoriticalGrid,
                              CT_RegularGridInt* beforeGrid,
                              int effectiveRayThresh );

    /*!
    *  \brief Computes the "best point of view" grid of a given set of "theoritical" and "before" grids
    *
    *  Each voxel of the resulting grid stores the index of the scanner that describes it the best
    *
    *  \param id : ID of the item drawable
    *  \param result : result to attach the grid
    *  \param bot : bottom left corner of the grid
    *  \param top : top left corner of the grid
    *  \param res : resolution(size of each voxel) of the grid
    *  \param theoriticalGridsList : list of the theoritical grids from the different scans
    *  \param beforeGridsList : list of the before grids from the different scans
    *
    *
    *  \return Returns the "best point of view" grid storing the index of the scanner that describes it the best
    */
    CT_RegularGridInt* computeBestPointOfViewGrid( qint64 id,
                                                   Result* result,
                                                   const CT_Point& bot,
                                                   const CT_Point& top,
                                                   float res,
                                                   QList<CT_RegularGridInt *> &theoriticalGridsList,
                                                   QList<CT_RegularGridInt *> &beforeGridsList,
                                                   QList<CT_RegularGridInt *> &HitsGridsList );

    /*!
    *  \brief Indicates which category a given intensity belongs to
    *
    *  We assume the _categoriesMarks has been filled with caution (increasing order)
    *
    *  \param intensity : intensity you want to know the category of
    *  \param nCategories : number of categories
    *  \param benchmarks : benchmarks between categories (must be in increasing order)
    *
    *  \return Returns the category containing this intensity
    */
    int getCategory ( float intensity, int nCategories, const vector<int>& benchmarks );

    /*!
    *  \brief Saves the density result of the step
    *
    *  Writes in an output file, for each voxel of the grid : "density Ni Nt Nb Ni_ith_category"
    *
    *  \param densityGrid : densityGrid computed by this step
    *  \param hitGrid : hitGrid computed by this step
    *  \param theoriticalGrid : theoriticalGrid computed by this step
    *  \param beforeGrid : beforeGrid computed by this step
    *  \param categoryHitGridList : list storing the category hit grids for each category
    *  \param nCategories : number of categories
    *  \param categoriesBenchmarks : thrsholds between categories
    *
    */
    void saveDensityOnlyResult ( CT_RegularGridDouble* densityGrid,
                                 CT_RegularGridInt* hitGrid,
                                 CT_RegularGridInt* theoriticalGrid,
                                 CT_RegularGridInt* beforeGrid,
                                 QList<CT_RegularGridInt *> categoryHitGridList,
                                 int nCategories,
                                 const vector<int>& categoriesBenchmarks,
                                 QString filePath,
                                 QString fileName );


    /*!
    *  \brief Saves the density result of the step
    *
    *  Writes in an output file, for each voxel of the grid : "density Ni Nt Nb Ni_ith_category"
    *
    *  \param densityGridList : list of densityGrid computed by this step
    *  \param hitGridList : list of hitGrid computed by this step
    *  \param theoriticalGridList : list of theoriticalGrid computed by this step
    *  \param beforeGridList : list of beforeGrid computed by this step
    *  \param categoryHitGridListList : list storing for each scanner the list of category hit grids for each category
    *  \param nCategories : number of categories
    *  \param categoriesBenchmarks : thrsholds between categories
    *
    */
    void saveDensityOnlyResult ( const QList< CT_RegularGridDouble* >& densityGridList,
                                 const QList< CT_RegularGridInt* >& hitGridList,
                                 const QList< CT_RegularGridInt* >& theoriticalGrid,
                                 const QList< CT_RegularGridInt* >& beforeGrid,
                                 const QList< QList< CT_RegularGridInt* > >& categoryHitGridListList,
                                 const CT_RegularGridInt* bestPointOfViewGrid,
                                 int nCategories,
                                 const vector<int>& categoriesBenchmarks,
                                 QString filePath, QString fileName );

    /*!
    *  \brief Saves the density result of the step
    *
    *  Writes in an output file, for each voxel of the grid : "density Ni Nt Nb Ni_ith_category"
    *
    *  \param densityGrid : densityGrid computed by this step
    *  \param hitGrid : hitGrid computed by this step
    *  \param theoriticalGrid : theoriticalGrid computed by this step
    *  \param beforeGrid : beforeGrid computed by this step
    *  \param deltaInGrid : deltaOutGrid computed by this step
    *  \param deltaOutGrid : deltaInGrid computed by this step
    *  \param categoryHitGridList : list storing the category hit grids for each category
    *  \param categoryDeltaOutGridList : list storing the category delta in grids for each category
    *  \param categoryDeltaInGridList : list storing the category delta out grids for each category
    *  \param nCategories : number of categories
    *  \param categoriesBenchmarks : thrsholds between categories
    *  \param filePath : where to store the result file
    *  \param fileName : name of the result file
    *
    */
    void saveDensityAndDistancesResult ( CT_RegularGridDouble* densityGrid,
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
                                         QString extension = "" );

    /*!
    *  \brief Saves the density result of the step
    */
    void saveDensityAndDistancesResult ( const QList< CT_RegularGridDouble* > & densityGridList,
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
                                         QString fileName );

}

#endif // COMPUTEGRIDTOOLS_H
