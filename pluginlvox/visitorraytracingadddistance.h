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

#ifndef VISITORRAYTRACINGADDDISTANCE_H
#define VISITORRAYTRACINGADDDISTANCE_H

/** \file visitorraytracingadddistance.h
    \author Ravaglia J. - mailto : joris.ravaglia@gmail.com
    \version 0.1
*/

#include "abstractvisitorraytracing.h"                          // Inherits from the AbstractVisitorRaytracing class
#include "ct_itemdrawable/ct_ray.h"                             // Needs a pointer to a ray to compute the distance to add

/** \class VisitorRaytracingAddDistance
    \brief This class is a specialization of the AbstractVisitorRaytracing class.
           It visits a voxel from a grid increasing the value associated to the voxel by the distance the ray spends in the voxels.

           This visitor should only be used to visit :
           \li \c AlignLeft CT_RegularGridInt
           \li \c AlignLeft CT_RegularGridFloat
           \li \c AlignLeft CT_RegularGridDouble
*/

class VisitorRaytracingAddDistance : public AbstractVisitorRaytracing
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
    VisitorRaytracingAddDistance();

    /*!
    *  \brief Constructor
    *
    *  \param ray : ray to compute the time spent in the voxel
    *
    */
    VisitorRaytracingAddDistance( CT_Ray* ray );

    /*!
    *  \brief Copy constructor
    *
    *  Copy constructor of the class
    *
    *  \param visitor : visitor to copy
    */
    VisitorRaytracingAddDistance( const VisitorRaytracingAddDistance& visitor );

    /*!
    *  \brief Destructor
    *
    *  Destructor of the class
    *
    */
    ~VisitorRaytracingAddDistance();

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
    VisitorRaytracingAddDistance& operator= (const VisitorRaytracingAddDistance& visitor);

//********************************************//
//                  Getters                   //
//********************************************//
    /*!
    *  \brief Getter of the class
    *
    *  \return Returns the bottom left corner of the factory
    */
    inline CT_Ray* getRay () const { return _ray; }

//********************************************//
//                  Setters                   //
//********************************************//
    /*!
    *  \brief Setter of the class
    */
    inline void setRay ( CT_Ray* ray ) { _ray = ray; }

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
    void visit(CT_AbstractRegularGridDataInterface *grid, int voxelID);

private :
    CT_Ray* _ray; /*!< ray to compute the time spent in the voxel*/
};

#endif // VISITORRAYTRACINGADDDISTANCE_H
