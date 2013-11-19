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

#include "lvox_visitorraytracingadddistance.h"

#include "ct_math/ct_mathpoint.h"                   // Used for distances between to points

LVOX_VisitorRaytracingAddDistance::LVOX_VisitorRaytracingAddDistance()
{
    _ray = NULL;
}

LVOX_VisitorRaytracingAddDistance::LVOX_VisitorRaytracingAddDistance(CT_Ray *ray)
{
    _ray = ray;
}

LVOX_VisitorRaytracingAddDistance::LVOX_VisitorRaytracingAddDistance(const LVOX_VisitorRaytracingAddDistance &visitor)
{
    _ray = visitor._ray;
}

LVOX_VisitorRaytracingAddDistance::~LVOX_VisitorRaytracingAddDistance()
{
    // Nothing to do, the ray must not be deleted
}

LVOX_VisitorRaytracingAddDistance& LVOX_VisitorRaytracingAddDistance::operator =( const LVOX_VisitorRaytracingAddDistance& visitor )
{
    _ray = visitor._ray;

    return (*this);
}

void LVOX_VisitorRaytracingAddDistance::visit(CT_AbstractRegularGridDataInterface *grid, int voxelID)
{
    CT_Point bot, top;
    float distanceToAdd = 0;

    // Getting the bbox of the voxel of interest
    grid->getBBox( voxelID, bot, top);

    // If the ray intersects the voxel
    CT_Point near, far;
    if ( _ray->intersect( bot, top, near, far) )
    {
        distanceToAdd = CT_MathPoint::distance3D( near, far );
    }

    grid->increment( voxelID, distanceToAdd );
}
