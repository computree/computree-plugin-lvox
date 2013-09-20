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

#include "visitorraytracingincrement.h"

VisitorRaytracingIncrement::VisitorRaytracingIncrement() : AbstractVisitorRaytracing()
{
    // Nothing to do
}

VisitorRaytracingIncrement::VisitorRaytracingIncrement(const VisitorRaytracingIncrement &visitor)
{
    // Nothing to do
}

VisitorRaytracingIncrement::~VisitorRaytracingIncrement()
{
    // Nothing to do
}

VisitorRaytracingIncrement& VisitorRaytracingIncrement::operator =(const VisitorRaytracingIncrement& visitor)
{
    // Nothing to do
}

void VisitorRaytracingIncrement::visit(CT_AbstractRegularGridDataInterface *grid, int voxelID)
{
    grid->increment(voxelID, 1);
}
