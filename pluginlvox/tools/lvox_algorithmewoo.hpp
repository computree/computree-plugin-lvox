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

#ifndef LVOX_ALGORITHMWOO_HPP
#define LVOX_ALGORITHMWOO_HPP

#include "lvox_algorithmewoo.h"
#include "ct_itemdrawable/ct_grid3d.h"

#include <QDebug>

template< typename DataT>
LVOX_AlgorithmWoo<DataT>::LVOX_AlgorithmWoo()
{
    _cptVoxels = 0;

    _currentVoxel.setX(0);
    _currentVoxel.setY(0);
    _currentVoxel.setZ(0);

    _stepAxis.setX(0);
    _stepAxis.setY(0);
    _stepAxis.setZ(0);

    _tMax.setX(0);
    _tMax.setY(0);
    _tMax.setZ(0);

    _tDel.setX(0);
    _tDel.setY(0);
    _tDel.setZ(0);

    _intersects = false;
}

template< typename DataT>
LVOX_AlgorithmWoo<DataT>::LVOX_AlgorithmWoo(const CT_Beam &ray, CT_Grid3D<DataT> *calibrationGrid)
{
    _cptVoxels = 0;

    QVector3D start;
    QVector3D end;

    QVector3D bot, top;
    calibrationGrid->getMinCoordinates(bot);
    calibrationGrid->getMaxCoordinates(top);
    float res = calibrationGrid->resolution();

    _intersects = ray.intersect(bot, top, start, end);

    if ( _intersects )
    {
        // Direction of the ray along each axis
        end.x() > start.x() ? _stepAxis.setX(1) : _stepAxis.setX(-1);
        end.y() > start.y() ? _stepAxis.setY(1) : _stepAxis.setY(-1);
        end.z() > start.z() ? _stepAxis.setZ(1) : _stepAxis.setZ(-1);

        // The voxel pierced by the first intersection between the ray and the grid
        if (!calibrationGrid->gridCoordinatesAtXYZ(start.x(), start.y(), start.z(), _currentVoxel))
        {
            qDebug() << "Error while initializing traversal algorithm : initial voxel is not part of the grid ";
            qDebug() << "Initial voxel (in the grid coordinate system) : " << _currentVoxel.x() << _currentVoxel.y() << _currentVoxel.z();
            qDebug() << "Starting point of the algorithm : " << start.x() << start.y() << start.z();
            qDebug() << "Bottom left corner of the calibration grid : " << bot.x() << bot.y() << bot.z();
            qDebug() << "Top right corner of the calibration grid : " << top.x() << top.y() << top.z();
            qDebug() << "Dimensions of the calibration grid : " << calibrationGrid->xdim() << calibrationGrid->ydim() << calibrationGrid->zdim();
            exit(1);
        }

        // Wether the ray is along the direction axis or opposite, the tMax value computation will differ (the first encountered boundary may be forward or backward)
        QVector3D boundary;
        _stepAxis.x() > 0 ?
                    boundary.setX(((_currentVoxel.x()+1)*res) + bot.x()) :
                    boundary.setX((_currentVoxel.x()*res) + bot.x());

        _stepAxis.y() > 0 ?
                    boundary.setY(((_currentVoxel.y()+1)*res) + bot.y()) :
                    boundary.setY((_currentVoxel.y()*res) + bot.y());

        _stepAxis.z() > 0 ?
                    boundary.setZ(((_currentVoxel.z()+1)*res) + bot.z()) :
                    boundary.setZ((_currentVoxel.z()*res) + bot.z());

        // Initializing the tMax values
        ray.getDirection().x() != 0 ?
                    _tMax.setX(fabs((boundary.x() - start.x()) / ray.getDirection().x())) :
                    _tMax.setX(LONG_MAX);

        ray.getDirection().y() != 0 ?
                    _tMax.setY(fabs((boundary.y() - start.y()) / ray.getDirection().y())) :
                    _tMax.setY(LONG_MAX);

        ray.getDirection().z() != 0 ?
                    _tMax.setZ(fabs((boundary.z() - start.z()) / ray.getDirection().z())) :
                    _tMax.setZ(LONG_MAX);

        // Initializing the deltaT values
        ray.getDirection().x() != 0 ?
                    _tDel.setX(fabs(res / ray.getDirection().x())) :
                    _tDel.setX(LONG_MAX);

        ray.getDirection().y() != 0 ?
                    _tDel.setY(fabs(res / ray.getDirection().y())) :
                    _tDel.setY(LONG_MAX);

        ray.getDirection().z() != 0 ?
                    _tDel.setZ(fabs(res / ray.getDirection().z())) :
                    _tDel.setZ(LONG_MAX);
    }
}

template< typename DataT>
LVOX_AlgorithmWoo<DataT>::~LVOX_AlgorithmWoo()
{
    // Nothing to do
}

template< typename DataT>
void LVOX_AlgorithmWoo<DataT>::reset(const CT_Beam &ray, CT_Grid3D<DataT> *calibrationGrid)
{
    _cptVoxels = 0;

    QVector3D start;
    QVector3D end;

    QVector3D bot, top;
    calibrationGrid->getMinCoordinates(bot);
    calibrationGrid->getMaxCoordinates(top);
    float res = calibrationGrid->resolution();

    _intersects = ray.intersect(bot, top, start, end);

    if ( _intersects )
    {
        // Direction of the ray along each axis
        end.x() > start.x() ? _stepAxis.setX(1) : _stepAxis.setX(-1);
        end.y() > start.y() ? _stepAxis.setY(1) : _stepAxis.setY(-1);
        end.z() > start.z() ? _stepAxis.setZ(1) : _stepAxis.setZ(-1);

        // The voxel pierced by the first intersection between the ray and the grid
        if ( !calibrationGrid->gridCoordinatesAtXYZ(start.x(), start.y(), start.z(), _currentVoxel))
        {           
            qDebug() << "Error while initializing traversal algorithm : initial voxel is not part of the grid ";
            qDebug() << "The starting point of the algorithm is not part of the grid";
            qDebug() << "Initial voxel (in the grid coordinate system) : " << _currentVoxel.x() << _currentVoxel.y() << _currentVoxel.z();
            qDebug() << "Starting point of the algorithm : " << start.x() << start.y() << start.z();
            qDebug() << "Bottom left corner of the calibration grid : " << bot.x() << bot.y() << bot.z();
            qDebug() << "Top right corner of the calibration grid : " << top.x() << top.y() << top.z();
            qDebug() << "Dimensions of the calibration grid : " << calibrationGrid->xdim() << calibrationGrid->ydim() << calibrationGrid->zdim();
            exit(1);
        }

        // Wether the ray is along the direction axis or opposite, the tMax value computation will differ (the first encountered boundary may be forward or backward)
        QVector3D boundary;
        _stepAxis.x() > 0 ?
                    boundary.setX(((_currentVoxel.x()+1)*res) + bot.x()) :
                    boundary.setX((_currentVoxel.x()*res) + bot.x());

        _stepAxis.y() > 0 ?
                    boundary.setY(((_currentVoxel.y()+1)*res) + bot.y()) :
                    boundary.setY((_currentVoxel.y()*res) + bot.y());


        _stepAxis.z() > 0 ?
                    boundary.setZ(((_currentVoxel.z()+1)*res) + bot.z()) :
                    boundary.setZ((_currentVoxel.z()*res) + bot.z());

        // Initializing the tMax values
        ray.getDirection().x() != 0 ?
                    _tMax.setX(fabs((boundary.x() - start.x()) / ray.getDirection().x())) :
                    _tMax.setX(LONG_MAX);

        ray.getDirection().y() != 0 ?
                    _tMax.setY(fabs((boundary.y() - start.y()) / ray.getDirection().y())) :
                    _tMax.setY(LONG_MAX);

        ray.getDirection().z() != 0 ?
                    _tMax.setZ(fabs((boundary.z() - start.z()) / ray.getDirection().z())) :
                    _tMax.setZ(LONG_MAX);

        // Initializing the deltaT values
        ray.getDirection().x() != 0 ?
                    _tDel.setX(fabs(res / ray.getDirection().x())) :
                    _tDel.setX(LONG_MAX);

        ray.getDirection().y() != 0 ?
                    _tDel.setY(fabs(res / ray.getDirection().y())) :
                    _tDel.setY(LONG_MAX);

        ray.getDirection().z() != 0 ?
                    _tDel.setZ(fabs(res / ray.getDirection().z())) :
                    _tDel.setZ(LONG_MAX);
    }
}

template< typename DataT>
qint64 LVOX_AlgorithmWoo<DataT>::run(const QList<CT_Grid3D<DataT> *> &gridsList, const QList<AbstractVisitorRaytracing *> &visitorsList)
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
        visitorsList[i]->visit( gridsList[i], gridsList[i]->index((int)_currentVoxel.x(), (int)_currentVoxel.y(), (int)_currentVoxel.z()));
    }
    _cptVoxels++;

    while ( 1 )
    {
        // Finds along which axis to do the next step
        int bits =	(( _tMax.x() < _tMax.y() ) << 2) +
                    (( _tMax.x() < _tMax.z() ) << 1) +
                    (( _tMax.y() < _tMax.z() ));
        nextStepAxis = chooseAxis[bits];

        // Going to next voxel along this direction
        if      (nextStepAxis==0) {_currentVoxel.setX(_currentVoxel.x() + _stepAxis.x());}
        else if (nextStepAxis==1) {_currentVoxel.setY(_currentVoxel.y() + _stepAxis.y());}
        else if (nextStepAxis==2) {_currentVoxel.setZ(_currentVoxel.z() + _stepAxis.z());}

        // Checks if the currentvoxel is outside the grid, the algorithm has finished
        if ( _currentVoxel.x() < 0 || _currentVoxel.x() >= gridsList[0]->xdim()) {return _cptVoxels;}
        if ( _currentVoxel.y() < 0 || _currentVoxel.y() >= gridsList[0]->ydim()) {return _cptVoxels;}
        if ( _currentVoxel.z() < 0 || _currentVoxel.z() >= gridsList[0]->zdim()) {return _cptVoxels;}

        // Visits the new voxel in each grid
        for ( unsigned int i = 0 ; i < listSize ; i++ )
        {
            visitorsList[i]->visit( gridsList[i], gridsList[i]->gridToIndex( _currentVoxel ) );
        }
        _cptVoxels++;

        // Updating tmax of this axis (increasing by deltaT)       
        if      (nextStepAxis==0) {_tMax.setX(_tMax.x() + _tDel.x());}
        else if (nextStepAxis==1) {_tMax.setY(_tMax.y() + _tDel.y());}
        else if (nextStepAxis==2) {_tMax.setZ(_tMax.z() + _tDel.z());}

    }

    // Unuseful, but diminishing the compilor warnings
    return _cptVoxels;
}

#endif // LVOX_ALGORITHMWOO_HPP
