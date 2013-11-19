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

/*! \def    EPSILON_INTERSECTION_RAY
            Redefinition of the zero for the ray-box intersection algorithm */
#define EPSILON_INTERSECTION_RAY 0.000001    // 10^-6

#include "lvox_computegridtools.h"
#include "qdebug.h"
#include "ct_itemdrawable/ct_beam.h"

void LVOX_ComputeGridTools::computeHitGrid(CT_Scanner *scanner,
                                           CT_Grid3D<int> *grilleHits,
                                           CT_Grid3D<double> *grilleIn,
                                           CT_Grid3D<double> *grilleOut,
                                           const CT_Scene *scene,
                                           double intensityThresh,
                                           bool greaterThan)
{      
    const CT_AbstractPointCloud *pointCloud = scene->getPointCloud();
    const CT_AbstractPointCloudIndex *pointCloudIndex = scene->getPointCloudIndex();
    quint64 n_points = pointCloudIndex->indexSize();
    QVector3D scanPos = scanner->getPosition();
    float res = grilleHits->resolution();

    for (quint64 i = 0 ; i < n_points; i++)
    {
        const int &index = (*pointCloudIndex)[i];
        const CT_Point &point = (*pointCloud)[index];
        int indice = grilleHits->indexAtXYZ(point.x, point.y, point.z);

        if (indice < 0)
        {
            qDebug() << "Le point "<< i << " de la scene n'est pas dans la grille";
        } else
        {
            // Hits Computing
            if    ((greaterThan && (point.intensity > intensityThresh))
                   || (!greaterThan && (point.intensity < intensityThresh)))
            {
                grilleHits->addValueAtIndex(indice, 1);

                // Distances Sum Computing
                QVector3D direction (point.x - scanPos.x(),
                                     point.y - scanPos.y(),
                                     point.z - scanPos.z());

                QVector3D bottom, top, in, out;
                grilleHits->getCellBottomLeftCornerAtXYZ(point.x, point.y, point.z, bottom);
                top.setX(bottom.x() + res);
                top.setY(bottom.y() + res);
                top.setZ(bottom.z() + res);

                CT_Beam beam(NULL, NULL, scanPos, direction);

                if (beam.intersect(bottom, top, in, out))
                {
                    double distanceIn = sqrt(pow(in.x()-point.x, 2) + pow(in.y()-point.y, 2) + pow(in.z()-point.z, 2));
                    double distanceOut = sqrt(pow(out.x()-point.x, 2) + pow(out.y()-point.y, 2) + pow(out.z()-point.z, 2));

                    grilleIn->addValueAtIndex(indice, distanceIn);
                    grilleOut->addValueAtIndex(indice, distanceOut);
                }
            }
        }
    }

    // Convert sums into means
    int ncells = grilleHits->nCells();
    for (int i = 0 ; i < ncells ; i++)
    {
        double value = grilleHits->valueAtIndex(i);
        int na = grilleHits->NA();
        if (value==0 || value==na)
        {
            grilleIn->setValueAtIndex(i, grilleIn->NA());
            grilleOut->setValueAtIndex(i, grilleOut->NA());
        } else {
            double invalue = grilleIn->valueAtIndex(i) / value;
            double outvalue = grilleOut->valueAtIndex(i) / value;

            grilleIn->setValueAtIndex(i, invalue);
            grilleOut->setValueAtIndex(i, outvalue);
        }
    }
    grilleHits->computeMinMax();
    grilleIn->computeMinMax();
    grilleOut->computeMinMax();
}
