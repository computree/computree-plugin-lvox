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

#include "lvox_visitorraytracingincrement.h"

LVOX_VisitorRaytracingIncrement::LVOX_VisitorRaytracingIncrement() : AbstractVisitorRaytracing()
{
    // Nothing to do
}

LVOX_VisitorRaytracingIncrement::LVOX_VisitorRaytracingIncrement(const LVOX_VisitorRaytracingIncrement &visitor)
{
    // Nothing to do
}

LVOX_VisitorRaytracingIncrement::~LVOX_VisitorRaytracingIncrement()
{
    // Nothing to do
}

LVOX_VisitorRaytracingIncrement& LVOX_VisitorRaytracingIncrement::operator =(const LVOX_VisitorRaytracingIncrement& visitor)
{
    // Nothing to do
}

void LVOX_VisitorRaytracingIncrement::visit(CT_AbstractRegularGridDataInterface *grid, int voxelID)
{
    grid->increment(voxelID, 1);
}
