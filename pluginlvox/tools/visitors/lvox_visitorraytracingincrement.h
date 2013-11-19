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

#ifndef LVOX_VISITORRAYTRACINGINCREMENT_H
#define LVOX_VISITORRAYTRACINGINCREMENT_H


#include "abstractvisitorraytracing.h"                          // Inherits from the AbstractVisitorRaytracing class

/** \class LVOX_VisitorRaytracingIncrement
    \brief This class is a specialization of the AbstractVisitorRaytracing class.
           It visits a voxel from a grid increasing the value associated to the voxel by 1.

           This visitor should only be used to visit :
           \li \c AlignLeft CT_RegularGridInt
           \li \c AlignLeft CT_RegularGridFloat
           \li \c AlignLeft CT_RegularGridDouble
*/
class LVOX_VisitorRaytracingIncrement : public AbstractVisitorRaytracing
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
    LVOX_VisitorRaytracingIncrement();

    /*!
    *  \brief Copy constructor
    *
    *  Copy constructor of the class
    *
    *  \param visitor : visitor to copy
    */
    LVOX_VisitorRaytracingIncrement( const LVOX_VisitorRaytracingIncrement& visitor );

    /*!
    *  \brief Destructor
    *
    *  Destructor of the class
    *
    */
    ~LVOX_VisitorRaytracingIncrement();

//********************************************//
//                Operators                   //
//********************************************//
    /*!
    *  \brief Assignment operator
    *
    *  Assignment operator of the class
    *
    *  \param visitor : visitor to copy
    */
    LVOX_VisitorRaytracingIncrement& operator= (const LVOX_VisitorRaytracingIncrement& visitor);

//********************************************//
//             Visiting method                //
//********************************************//
    /*!
    *  \brief Visiting method
    *
    *  Increment the data of a specific voxel in a given grid
    *
    *  \param grid : the grid to modify
    *  \param voxelID : the index of the voxel that needs to be incremented
    */
    virtual void visit(CT_AbstractRegularGridDataInterface *grid, int voxelID);

private :
};

#endif // LVOX_VISITORRAYTRACINGINCREMENT_H
