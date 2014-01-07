/****************************************************************************

 Copyright (C) 2012-2012 Université de Sherbrooke, Québec, CANADA
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

#include "algorithmewoo.h"

#include <QDebug>

AlgorithmWoo::AlgorithmWoo()
{
    _cptVoxels = 0;

    for ( int i = 0 ; i < 3 ; i++ )
    {
        _currentVoxel.data[i] = 0;
        _stepAxis.data[i] = 0;
        _tMax.data[i] = 0;
        _tDel.data[i] = 0;
    }

    _intersects = false;
}

AlgorithmWoo::AlgorithmWoo(const CT_Ray &ray, CT_AbstractRegularGridDataInterface *calibrationGrid)
{
    _cptVoxels = 0;

    CT_Point start;
    CT_Point end;

    _intersects = ray.intersect(calibrationGrid, start, end);

    if ( _intersects )
    {
        // Direction of the ray along each axis
        for ( int i = 0 ; i < 3 ; i++ )
            end.data[i] > start.data[i] ? _stepAxis.data[i] = 1 : _stepAxis.data[i] = -1;

        // The voxel pierced by the first intersection between the ray and the grid
        if ( !calibrationGrid->spaceToGrid( start, _currentVoxel ) )
        {
            qDebug() << "Error while initializing traversal algorithm : initial voxel is not part of the grid ";
            qDebug() << "Initial voxel (in the grid coordinate system) : " << _currentVoxel.x << _currentVoxel.y << _currentVoxel.z;
            qDebug() << "Starting point of the algorithm : " << start.x << start.y << start.z;
            qDebug() << "Bottom left corner of the calibration grid : " << calibrationGrid->getBot().x << calibrationGrid->getBot().y << calibrationGrid->getBot().z;
            qDebug() << "Top right corner of the calibration grid : " << calibrationGrid->getTop().x << calibrationGrid->getTop().y << calibrationGrid->getTop().z;
            qDebug() << "Dimensions of the calibration grid : " << calibrationGrid->getDim().x << calibrationGrid->getDim().y << calibrationGrid->getDim().z;
            exit(1);
        }

        // Wether the ray is along the direction axis or opposite, the tMax value computation will differ (the first encountered boundary may be forward or backward)
        CT_Point boundary;
        for ( int i = 0 ; i < 3 ; i++ )
            _stepAxis.data[i] > 0 ? boundary.data[i] = ((_currentVoxel.data[i]+1)*calibrationGrid->getRes()) + calibrationGrid->getBot().data[i] : boundary.data[i] = (_currentVoxel.data[i]*calibrationGrid->getRes()) + calibrationGrid->getBot().data[i];

        // Initialisation des tMax
        for ( unsigned int i = 0 ; i < 3 ; i ++ )
        {
            // Initializing the tMax values
            ray.getDirection().data[i] != 0 ? _tMax.data[i] = fabs((boundary.data[i] - start.data[i]) / ray.getDirection().data[i]) : _tMax.data[i] = LONG_MAX;

            // Initializing the deltaT values
            ray.getDirection().data[i] != 0 ? _tDel.data[i] = fabs(calibrationGrid->getRes() / ray.getDirection().data[i]) : _tDel.data[i] = LONG_MAX;
        }
    }
}

AlgorithmWoo::AlgorithmWoo(const AlgorithmWoo &algorithm)
{
    _cptVoxels = algorithm._cptVoxels;
    _currentVoxel = algorithm._currentVoxel;
    _stepAxis = algorithm._stepAxis;
    _tMax = algorithm._tMax;
    _tDel = algorithm._tDel;
    _intersects = algorithm._intersects;
}

AlgorithmWoo::~AlgorithmWoo()
{
    // Nothing to do
}

void AlgorithmWoo::reset(const CT_Ray &ray, CT_AbstractRegularGridDataInterface *calibrationGrid)
{
    _cptVoxels = 0;

    CT_Point start;
    CT_Point end;

    _intersects = ray.intersect(calibrationGrid, start, end);

    if ( _intersects )
    {
        // Direction of the ray along each axis
        for ( int i = 0 ; i < 3 ; i++ )
            end.data[i] > start.data[i] ? _stepAxis.data[i] = 1 : _stepAxis.data[i] = -1;

        // The voxel pierced by the first intersection between the ray and the grid
        if ( !calibrationGrid->spaceToGrid( start, _currentVoxel ) )
        {
            qDebug() << "\nError while reinitializing traversal algorithm : ";
            qDebug() << "The starting point of the algorithm is not part of the grid";
            qDebug() << "Initial voxel (in the grid coordinate system) : " << _currentVoxel.x << _currentVoxel.y << _currentVoxel.z;
            qDebug() << "Starting point of the algorithm : " << start.x << start.y << start.z;
            qDebug() << "Bottom left corner of the calibration grid : " << calibrationGrid->getBot().x << calibrationGrid->getBot().y << calibrationGrid->getBot().z;
            qDebug() << "Top right corner of the calibration grid : " << calibrationGrid->getTop().x << calibrationGrid->getTop().y << calibrationGrid->getTop().z;
            qDebug() << "Dimensions of the calibration grid : " << calibrationGrid->getDim().x << calibrationGrid->getDim().y << calibrationGrid->getDim().z;
            exit(1);
        }

        // Wether the ray is along the direction axis or opposite, the tMax value computation will differ (the first encountered boundary may be forward or backward)
        CT_Point boundary;
        for ( int i = 0 ; i < 3 ; i++ )
            _stepAxis.data[i] > 0 ? boundary.data[i] = ((_currentVoxel.data[i]+1)*calibrationGrid->getRes()) + calibrationGrid->getBot().data[i] : boundary.data[i] = (_currentVoxel.data[i]*calibrationGrid->getRes()) + calibrationGrid->getBot().data[i];

        // Initialisation des tMax
        for ( unsigned int i = 0 ; i < 3 ; i ++ )
        {
            // Initializing the tMax values
            ray.getDirection().data[i] != 0 ? _tMax.data[i] = fabs((boundary.data[i] - start.data[i]) / ray.getDirection().data[i]) : _tMax.data[i] = LONG_MAX;

            // Initializing the deltaT values
            ray.getDirection().data[i] != 0 ? _tDel.data[i] = fabs(calibrationGrid->getRes() / ray.getDirection().data[i]) : _tDel.data[i] = LONG_MAX;
        }
    }
}

qint64 AlgorithmWoo::run(const QList<CT_AbstractRegularGridDataInterface *> &gridsList, const QList<AbstractVisitorRaytracing *> &visitorsList)
{
    if ( !_intersects )
    {
        return 0;
    }

    // Make sure there is one visitor per grid
    assert( gridsList.size() == visitorsList.size() );
    unsigned int listSize = visitorsList.size();

    int		chooseAxis[8] = { 2, 1, 2, 1, 2, 2, 0, 0 };
    int		nextStepAxis;

    // Visits the first voxel in each grid
    for ( unsigned int i = 0 ; i < listSize ; i++ )
    {
        visitorsList[i]->visit( gridsList[i], gridsList[i]->gridToIndex( _currentVoxel ) );
    }
    _cptVoxels++;

    while ( 1 )
    {
        // Finds along which axis to do the next step
        int bits =	(( _tMax.x < _tMax.y ) << 2) +
                    (( _tMax.x < _tMax.z ) << 1) +
                    (( _tMax.y < _tMax.z ));
        nextStepAxis = chooseAxis[bits];

        // Going to next voxel along this direction
        _currentVoxel.data[ nextStepAxis ] += _stepAxis.data[ nextStepAxis ];

        // Checks if the currentvoxel is outside the grid, the algorithm has finished
        for ( int i = 0 ; i < 3 ; i++ )
        {
            if ( _currentVoxel.data[i] < 0 || _currentVoxel.data[i] >= gridsList[0]->getDim().data[i] )
            {
                return _cptVoxels;
            }
        }

        // Visits the new voxel in each grid
        for ( unsigned int i = 0 ; i < listSize ; i++ )
        {
            visitorsList[i]->visit( gridsList[i], gridsList[i]->gridToIndex( _currentVoxel ) );
        }
        _cptVoxels++;

        // Updating tmax of this axis (increasing by deltaT)
        _tMax.data[ nextStepAxis ] +=_tDel.data[ nextStepAxis ];
    }

    // Unuseful, but diminishing the compilor warnings
    return _cptVoxels;
}
