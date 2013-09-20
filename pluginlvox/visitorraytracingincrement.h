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

#ifndef VISITORRAYTRACINGINCREMENT_H
#define VISITORRAYTRACINGINCREMENT_H

/** \file visitorraytracingincrement.h
    \author Ravaglia J. - mailto : joris.ravaglia@gmail.com
    \version 0.1
*/

#include "abstractvisitorraytracing.h"                          // Inherits from the AbstractVisitorRaytracing class

/** \class VisitorRaytracingIncrement
    \brief This class is a specialization of the AbstractVisitorRaytracing class.
           It visits a voxel from a grid increasing the value associated to the voxel by 1.

           This visitor should only be used to visit :
           \li \c AlignLeft CT_RegularGridInt
           \li \c AlignLeft CT_RegularGridFloat
           \li \c AlignLeft CT_RegularGridDouble
*/
class VisitorRaytracingIncrement : public AbstractVisitorRaytracing
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
    VisitorRaytracingIncrement();

    /*!
    *  \brief Copy constructor
    *
    *  Copy constructor of the class
    *
    *  \param visitor : visitor to copy
    */
    VisitorRaytracingIncrement( const VisitorRaytracingIncrement& visitor );

    /*!
    *  \brief Destructor
    *
    *  Destructor of the class
    *
    */
    ~VisitorRaytracingIncrement();

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
    VisitorRaytracingIncrement& operator= (const VisitorRaytracingIncrement& visitor);

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

#endif // VISITORRAYTRACINGINCREMENT_H
