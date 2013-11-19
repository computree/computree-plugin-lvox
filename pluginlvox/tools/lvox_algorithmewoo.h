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

#ifndef LVOX_ALGORITHMEWOO_H
#define LVOX_ALGORITHMEWOO_H

#include "ct_itemdrawable/ct_beam.h"
#include "abstractvisitorraytracing.h"
#include "qvector3d.h"

template< typename DataT > class CT_Grid3D;

/** \class  LVOX_AlgorithmWoo
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
template< typename DataT>
class LVOX_AlgorithmWoo
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
    LVOX_AlgorithmWoo();

    /*!
    *  \brief Constructor
    *
    *  Constructor of the class
    *
    *  \param ray : ray that traverses the grid
    *  \param calibrationGrid : any grid that can provide the following parameters : bot, top, resolution and the spaceToVoxel method
    */
    LVOX_AlgorithmWoo( const CT_Beam& ray, CT_Grid3D<DataT>* calibrationGrid );

    /*!
    *  \brief Destructor
    *
    *  Destructor of the class
    *
    */
    ~LVOX_AlgorithmWoo();

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
    void reset(const CT_Beam& ray, CT_Grid3D<DataT> *calibrationGrid );

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
    qint64 run ( const QList< CT_Grid3D<DataT>* >& gridsList, const QList < AbstractVisitorRaytracing* >& visitorsList );


private :
    qint64      _cptVoxels;			/*!< number of voxel hit by the ray during the algorithm*/
    QVector3D	_currentVoxel;		/*!< current voxel all along the algorithm (grid coordinate system)*/
    QVector3D	_stepAxis;			/*!< indicates for each axis wether the ray goes forward (in the same direction than the base vector => 1) or backward (the opposite direction => -1)*/
    QVector3D    _tMax;             /*!< "the value of t at which the ray crosses the first voxel boundary (along each direction)"*/
    QVector3D    _tDel;             /*!< "how far along the ray we must move (in units of t)" for each component "of such a movement to equal the width of a voxel"*/
    bool        _intersects;        /*!< When creating or resetting the algorithm, this value indicates wether the ray intersects the box or not. If not, the run method won't do anything.*/
};

#include "tools/lvox_algorithmewoo.hpp"

#endif // LVOX_ALGORITHMEWOO_H
