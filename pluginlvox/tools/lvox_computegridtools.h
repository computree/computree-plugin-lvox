/****************************************************************************

 Copyright (C) 2012-2012 Universite de Sherbrooke, Quebec, CANADA
                     All rights reserved.

 Contact :  richard.fournier@usherbrooke.ca
            jean-francois.cote@nrcan-rncan.gc.ca
            joris.ravaglia@gmail.com

 Developers : Joris RAVAGLIA

 Adapted to Computree 2.0 by Alexandre PIBOULE

 This file is part of LVox plugin for Computree.

 LVox plugin is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 LVox plugin is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with LVox plugin.  If not, see <http://www.gnu.org/licenses/>.

*****************************************************************************/

#ifndef LVOX_COMPUTEGRIDTOOLS_H
#define LVOX_COMPUTEGRIDTOOLS_H

#include "ct_itemdrawable/ct_regulargridint.h"      // Using integers regular grids
#include "ct_itemdrawable/ct_regulargriddouble.h"   // Using doubles regular grids

#include "ct_itemdrawable/ct_scanner.h"             // Using scanners to compute grids

#include "ct_itemdrawable/ct_scene.h"               // Using scenes to compute grids
#include "ct_itemdrawable/ct_grid3d.h"

#include "qvector3d.h"


class LVOX_ComputeGridTools
{
public:

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
    static void computeHitGrid(CT_Grid3D<int> *grilleHits,
                                const CT_Scene* scene,
                                double intensityThresh,
                                bool greaterThan );

};

#endif // LVOX_COMPUTEGRIDTOOLS_H
