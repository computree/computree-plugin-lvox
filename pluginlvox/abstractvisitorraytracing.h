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

#ifndef ABSTRACTVISITORRAYTRACING_H
#define ABSTRACTVISITORRAYTRACING_H

/** \file abstractvisitorraytracing.h
    \author Ravaglia J. - mailto : joris.ravaglia@gmail.com
    \version 0.1
*/

#include "ct_itemdrawable/abstract/ct_abstractregulargriddatainterface.h"        // A visitor is visiting some regular grids

/** \class AbstractVisitorRaytracing
    \brief This class is part of the design pattern "visitor" used to separate the algorithm and its effects.
           A visitor is an obect modifier (in this case, a grid modifier) called by the raytracing algorithm on a grid.
           Since  we might use visitors for other purposes than raytracing, this class is the abstract class of the raytracing visitors only.

           A description of this pattern can be found at :
           http://en.wikipedia.org/wiki/Visitor_pattern
*/
class AbstractVisitorRaytracing
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
    AbstractVisitorRaytracing();

    /*!
    *  \brief Visiting method
    *
    *  Modifies the data of a specific voxel in a given grid
    *
    *  \param grid : the grid to modify
    *  \param voxelID : the index of the voxel that needs to be incremented
    */
    virtual void visit( CT_AbstractRegularGridDataInterface* grid, int voxelID ) = 0;

private :
};

#endif // ABSTRACTVISITORRAYTRACING_H
