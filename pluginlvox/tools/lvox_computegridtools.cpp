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

#include "lvox_computegridtools.h"
#include "qdebug.h"

void LVOX_ComputeGridTools::computeHitGrid(CT_Grid3D<int> *grilleHits, const CT_Scene* scene, double intensityThresh, bool greaterThan)
{      
    const CT_AbstractPointCloud *pointCloud = scene->getPointCloud();
    const CT_AbstractPointCloudIndex *pointCloudIndex = scene->getPointCloudIndex();
    int n_points = pointCloudIndex->indexSize();

    for (quint64 i = 0 ; i < n_points; i++)
    {
        const int &index = (*pointCloudIndex)[i];
        const CT_Point &point = (*pointCloud)[index];

        if    ((greaterThan && (point.intensity > intensityThresh))
           || (!greaterThan && (point.intensity < intensityThresh)))
        {
            if (!grilleHits->addValueAtXYZ(point.x, point.y, point.z, 1))
            {
                qDebug() << "Le point "<< i << " de la scene n'est pas dans la grille";
            }
        }
    }
}
