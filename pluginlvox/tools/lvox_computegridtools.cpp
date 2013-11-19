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

/*! \def    EPSILON_INTERSECTION_RAY
            Redefinition of the zero for the ray-box intersection algorithm */
#define EPSILON_INTERSECTION_RAY 0.000001    // 10^-6

#include "lvox_computegridtools.h"
#include "qdebug.h"

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
    int n_points = pointCloudIndex->indexSize();
    QVector3D scanPos = scanner->getPosition();

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
                // Normalizing direction
                direction.normalize();

                QVector3D bottom, in, out;
                grilleHits->getCellBottomLeftCornerAtXYZ(point.x, point.y, point.z, bottom);

                if (intersectsRay(scanPos, direction, bottom, grilleHits->resolution(), in, out))
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

bool LVOX_ComputeGridTools::intersectsRay(const QVector3D &scanCenter,
                                          const QVector3D &scanDirection,
                                          const QVector3D &bottomCorner,
                                          const float &res,
                                          QVector3D &inPoint,
                                          QVector3D &outPoint)
{

    double t0 = 0;
    double t1 = LONG_MAX;

    if (!LVOX_ComputeGridTools::updateIntervals( bottomCorner.x(),
                                                 bottomCorner.x() + res,
                                                 scanCenter.x(),
                                                 scanDirection.x(),
                                                 t0, t1)) {return false;}

    if (!LVOX_ComputeGridTools::updateIntervals( bottomCorner.y(),
                                                 bottomCorner.y() + res,
                                                 scanCenter.y(),
                                                 scanDirection.y(),
                                                 t0, t1)) {return false;}

    if (!LVOX_ComputeGridTools::updateIntervals( bottomCorner.z(),
                                                 bottomCorner.z() + res,
                                                 scanCenter.z(),
                                                 scanDirection.z(),
                                                 t0, t1)) {return false;}

    inPoint.setX(scanCenter.x() + scanDirection.x()*t0);
    inPoint.setY(scanCenter.y() + scanDirection.y()*t0);
    inPoint.setZ(scanCenter.z() + scanDirection.z()*t0);

    outPoint.setX(scanCenter.x() + scanDirection.x()*t1);
    outPoint.setY(scanCenter.y() + scanDirection.y()*t1);
    outPoint.setZ(scanCenter.z() + scanDirection.z()*t1);

    return true;
}

bool LVOX_ComputeGridTools::updateIntervals(const double &bc,
                                            const double &uc,
                                            const double &sc,
                                            const double &sd,
                                            double &t0, double &t1)
{
    // Update interval for bounding box slab
    double invRayDir = 1.f / sd;
    double tNear = (double)(bc - sc) * invRayDir;
    double tFar  = (double)(uc - sc) * invRayDir;

    // Update parametric interval from slab intersection $t$s
    if (tNear > tFar) std::swap(tNear, tFar);

    t0 = tNear > t0 ? tNear : t0;
    t1 = tFar  < t1 ? tFar  : t1;

    if (t0 > t1 && t0 - t1 > EPSILON_INTERSECTION_RAY ) // t0 being always > t1, (t0-t1) is always positive
    {
        return false;
    }

    return true;
}
