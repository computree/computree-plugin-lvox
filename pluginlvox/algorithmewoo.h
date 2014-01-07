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

#ifndef ALGORITHMEWOO_H
#define ALGORITHMEWOO_H

/** \file algorithmewoo.h
    \author J. Ravaglia - mail to : joris.ravaglia@gmail.com
    \version 0.1
*/

#include "ct_itemdrawable/abstract/ct_abstractregulargriddatainterface.h"        // The algorithm traverses a grid
#include "ct_itemdrawable/ct_ray.h"                                     // The algorithm is ray traversal and so needs an input ray
#include "abstractvisitorraytracing.h"                                  // Effects on the grids are applied by visitors

/** \class  AlgorithmWoo
    \brief  A ray traversal algorithm developped by Amanatides & Woo in 1987

    The article presenting this algorithm can be found at : http://www.cse.yorku.ca/~amana/research/grid.pdf

    Here is the BibTeX citation of the article :
    @inproceedings{ amanatides1987fast,
                    title={A fast voxel traversal algorithm for ray tracing},
                    author={Amanatides, J. and Woo, A.},
                    booktitle={Proceedings of EUROGRAPHICS},
                    volume={87},
                    pages={3--10},
                    year={1987}
                    }

*/
class AlgorithmWoo
{
public:
//********************************************//
//         Constructors/Destructors           //
//********************************************//
    /*!
    *  \brief Default constructor
    *
    *  Default constructor of the class
    *  Each attribute will be set to 0 or NULL
    *  Each vector will be empty
    *
    */
    AlgorithmWoo();

    /*!
    *  \brief Constructor
    *
    *  Constructor of the class
    *
    *  \param ray : ray that traverses the grid
    *  \param calibrationGrid : any grid that can provide the following parameters : bot, top, resolution and the spaceToVoxel method
    */
    AlgorithmWoo( const CT_Ray& ray, CT_AbstractRegularGridDataInterface* calibrationGrid );

    /*!
    *  \brief Copy constructor
    *
    *  Copy constructor of the class
    *
    *  \param algorithm : algorithm to copy
    */
    AlgorithmWoo( const AlgorithmWoo& algorithm );

    /*!
    *  \brief Destructor
    *
    *  Destructor of the class
    *
    */
    ~AlgorithmWoo();

//********************************************//
//                  PRIVATE                   //
//********************************************//
//                    Tools                   //
//********************************************//
    /*!
    *  \brief Reinitializes the algorithm with new parameters
    *
    *  Copy of the constructor
    *
    *  \param ray : ray that traverses the grid
    *  \param calibrationGrid : any grid that can provide the following parameters : bot, top, resolution and the spaceToVoxel method
    */
    void reset( const CT_Ray& ray, CT_AbstractRegularGridDataInterface* calibrationGrid );

//private :
//    /*!
//    *  \brief Runs the algorithm
//    *
//    *  Runs the algorithm over the uniform grid and calls the modifier method from the visitor on each traversed voxel
//    *  \warning The gridsList and the visitorsList have to be filled in a same way : the ith grid will be visited by the ith visitor
//    *
//    *  \param gridsList : list of grids the algorithms applies effects on
//    *  \param visitorsList : list of visitors applying effects on the grids
//    *
//    *  \return Returns the number of traversed voxels
//    */
//    int validateNextAxis ( int nextStepAxis, CT_Point& stepAxis, bool* validMove,  CT_AbstractRegularGridDataInterface* grid, AbstractVisitorRaytracing* visitor );

//********************************************//
//                    Run                     //
//********************************************//
    /*!
    *  \brief Runs the algorithm
    *
    *  Runs the algorithm over the uniform grid and calls the modifier method from the visitor on each traversed voxel
    *  \warning The gridsList and the visitorsList have to be filled in a same way : the ith grid will be visited by the ith visitor
    *
    *  \param gridsList : list of grids the algorithms applies effects on
    *  \param visitorsList : list of visitors applying effects on the grids
    *
    *  \return Returns the number of traversed voxels
    */
    qint64 run ( const QList< CT_AbstractRegularGridDataInterface* >& gridsList, const QList < AbstractVisitorRaytracing* >& visitorsList );

//    /*!
//    *  \brief Runs the algorithm
//    *
//    *  Runs the algorithm over the uniform grid and calls the modifier method from the visitor on each traversed voxel
//    *  \warning The gridsList and the visitorsList have to be filled in a same way : the ith grid will be visited by the ith visitor
//    *
//    *  \param gridsList : list of grids the algorithms applies effects on
//    *  \param visitorsList : list of visitors applying effects on the grids
//    *
//    *  \return Returns the number of traversed voxels
//    */
//    qint64 runPrecise ( const QList< CT_AbstractRegularGridDataInterface* >& gridsList, const QList < AbstractVisitorRaytracing* >& visitorsList );

private :
    qint64      _cptVoxels;			/*!< number of voxel hit by the ray during the algorithm*/
    CT_Point	_currentVoxel;		/*!< current voxel all along the algorithm (grid coordinate system)*/
    CT_Point	_stepAxis;			/*!< indicates for each axis wether the ray goes forward (in the same direction than the base vector => 1) or backward (the opposite direction => -1)*/
    CT_Point    _tMax;              /*!< "the value of t at which the ray crosses the first voxel boundary (along each direction)"*/
    CT_Point    _tDel;              /*!< "how far along the ray we must move (in units of t)" for each component "of such a movement to equal the width of a voxel"*/
    bool        _intersects;        /*!< When creating or resetting the algorithm, this value indicates wether the ray intersects the box or not. If not, the run method won't do anything.*/
};

#endif // ALGORITHMEWOO_H
