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

#include "gridfactory.h"
#include <limits>

GridFactory::GridFactory()
{
    _bot = createCtPoint(0,0,0);
    _top = createCtPoint(0,0,0);
    _dim = createCtPoint(0,0,0);
    _res = 0;
    _lastAllocatedIndex = 0;
}

GridFactory::GridFactory( const CT_Point& bot, const CT_Point& top, float resolution )
{
    assert( resolution > 0 );
    assert ( top.x > bot.x && top.y > bot.y && top.z > bot.z );

    _bot = bot;
    _top = top;
    _res = resolution;
    _lastAllocatedIndex = 0;

    // Compute dimension of the grid
    _dim.x = ceil ( ( _top.x - _bot.x ) / _res );
    _dim.y = ceil ( ( _top.y - _bot.y ) / _res );
    _dim.z = ceil ( ( _top.z - _bot.z ) / _res );
}

GridFactory::GridFactory(const GridFactory &gridFactory)
{
    _bot = gridFactory._bot;
    _top = gridFactory._top;
    _dim = gridFactory._dim;
    _res = gridFactory._res;
    _lastAllocatedIndex = gridFactory._lastAllocatedIndex;
}

GridFactory::~GridFactory()
{
    // Nothing to do
}

GridFactory& GridFactory::operator =(const GridFactory& gridFactory)
{
    _bot = gridFactory._bot;
    _top = gridFactory._top;
    _dim = gridFactory._dim;
    _res = gridFactory._res;
    _lastAllocatedIndex = gridFactory._lastAllocatedIndex;

    return (*this);
}

bool GridFactory::nextPartsOfGrids( QList<CT_AbstractRegularGridDataInterface *> &gridList )
{
    float	nMax = _dim.x*_dim.y*_dim.z - _lastAllocatedIndex;	// Total number of voxel to be allocated by this factory

    // May happen if the dimensions of the global grid are too big
    assert( isfinite(nMax) && nMax >= 0 );

    if ( nMax == 0 )
    {
        // All voxels of the general grid have already been allocated
        _lastAllocatedIndex--;	// For ulterior access to the last voxel of the gridfactory (it will be set to the last index)

//        // Clear all the grids ?
//        int listSize = gridList.size();
//        for ( unsigned int i = 0 ; i < listSize ; i++ )
//        {
//        }

        return false;
    }

    // Try to allocate less and less memory until the system accepts one allocation
    bool        stopTests = false;                          // When to stop the allocation tests
    float       nbVoxelAllocatable = nMax;					// Number of allocatable voxels
    float         realNVoxels;                                // Real number of voxel in the section
    CT_Point    sectionBot;
    CT_Point    sectionTop;
    CT_Point    sectionDim;


    while ( !stopTests )
    {
        stopTests = true;

        // Get the next section with the current parameters
        int oldAllocated = _lastAllocatedIndex;
        getSectionBBox( nbVoxelAllocatable, realNVoxels, sectionBot, sectionTop, sectionDim );

        int     listSize = gridList.size();
        bool    stopTry = false;
        try
        {
            for ( unsigned int i = 0 ; i < listSize && !stopTry ; i++ )
            {
                try
                {
                    gridList[i]->reset(sectionTop, sectionBot, _res);
                }

                catch( std::bad_alloc )
                {
                    // Clear all the grids in the list
                    for ( unsigned int j = 0 ; j < listSize ; j++ )
                    {
                        gridList[j]->clear();
                    }

                    // Diminishing the size of memory required
                    nbVoxelAllocatable *= 3.0/4.0;

                    // Restore the old _lastAllocatedIndex (which doesn't have to be changed since no allocation have been done)
                    _lastAllocatedIndex = oldAllocated;

                    stopTry = true;
                    stopTests = false;
                }
            }
        }

        catch( std::bad_alloc )
        {
            // Clear all the grids in the list
            for ( unsigned int j = 0 ; j < listSize ; j++ )
            {
                gridList[j]->clear();
            }

            nbVoxelAllocatable *= 3.0/4.0;
            stopTry = true;
            stopTests = false;
        }
    }

    return true;
}

void GridFactory::getSectionBBox(float nVoxels, float &outputNVoxels, CT_Point &outputBot, CT_Point &outputTop, CT_Point &outputDimensions)
{
    // Gets the bottom left point of the section bounding box (the bottom left point from the _lastAllocated voxel)
    // See the indexToGrid method from the CT_AbstractRegularGridData class
    CT_Point gridCoord;
    gridCoord.x = (int) ( _lastAllocatedIndex / (_dim.z*_dim.y) );

    if ( gridCoord.x != 0 )
        gridCoord.y = (int) ( (_lastAllocatedIndex % (int)(_dim.z*_dim.y)) / _dim.z ) ;
    else
        gridCoord.y = (int) ( _lastAllocatedIndex / _dim.z );

    if ( gridCoord.y != 0 )
        gridCoord.z = (_lastAllocatedIndex % (int)(_dim.z*_dim.y)) % (int)_dim.z;
    else if ( gridCoord.x == 0 )
        gridCoord.z = _lastAllocatedIndex;
    else
        gridCoord.z = (int) ( _lastAllocatedIndex % (int)_dim.z );

    for ( unsigned int i = 0 ; i < 3 ; i++ )
    {
        outputBot.data[i] = gridCoord.data[i] * _res + _bot.data[i];
    }

    // If the number of voxel is lesser than the number of voxel in a column OR the last allocated voxel is not at the bottom of a column
    if (nVoxels <= _dim.z || gridCoord.z != 0 )
    {
        // Make sure numbers are not too big
        assert ( INT_MAX / min(_dim.z - gridCoord.z, (float)nVoxels) > 1 );

        outputTop.x = outputBot.x + _res;
        outputTop.y = outputBot.y + _res;
        outputTop.z = outputBot.z + ( min(_dim.z - gridCoord.z, (float)nVoxels) * _res );

        outputDimensions.x = 0;
        outputDimensions.y = 0;
        outputDimensions.z = outputNVoxels;

        // Updates the ending index of the factory and the outputs
        outputNVoxels = min ((_dim.z - gridCoord.z), nVoxels);
        _lastAllocatedIndex += outputNVoxels;
    }

    // If the number of voxel is lesser than the number of voxels in a plane OR the last allocated voxel is in not at the begening of a plane
    else if ( nVoxels <= (_dim.z * _dim.y) || gridCoord.y != 0 )
    {
        // There is enough space to allocate more than one column
        // But not enough space is available to allocate an entire plan of voxels
        // The startVoxel should be at the bottom of a column otherwise it won't be possible to create a
        // section which is plane and which bbox is full of voxels
        assert(gridCoord.z == 0);
        // Make sure numbers are not too big
        assert ( INT_MAX / min(_dim.y - gridCoord.y, nVoxels / _dim.z) > 1 ); // Works because all values are positive

        outputTop.x = outputBot.x + _res;
        outputTop.y = outputBot.y + ( min(_dim.y - gridCoord.y, nVoxels / _dim.z) * _res );
        outputTop.z = outputBot.z + ( _dim.z * _res );

        outputDimensions.x = 0;
        outputDimensions.y = min(_dim.y - gridCoord.y,(float) nVoxels / _dim.z);
        outputDimensions.z = _dim.z;

        // Updates the ending index of the factory and the outputs
        outputNVoxels = min ((_dim.y - gridCoord.y), nVoxels) * _dim.z;
        _lastAllocatedIndex += outputNVoxels;
    }

    else
    {
        // There is enough space to allocate more than one plane
        // Allocate the biggest box
        // The startVoxel should be at the bottom of a column and at the begening of a plane otherwise it won't be possible to create a
        // section which forms a box which is full of voxels
        assert(gridCoord.z == 0);
        assert(gridCoord.y == 0);
        // Make sure numbers are not too big to calculate products
        assert ( INT_MAX / _dim.y > _dim.z );
        assert ( INT_MAX / min((_dim.x - gridCoord.x), nVoxels ) > (_dim.y * _dim.z) );

        outputTop.x = outputBot.x + ( min((_dim.x - gridCoord.x), nVoxels / (_dim.z * _dim.y)) * _res );
        outputTop.y = outputBot.y + ( _dim.y * _res );
        outputTop.z = outputBot.z + ( _dim.z * _res );

        outputDimensions.x = min((_dim.x - gridCoord.x), nVoxels / (_dim.z * _dim.y));
        outputDimensions.y = _dim.y;
        outputDimensions.z = _dim.z;

        // Updates the ending index of the factory
        outputNVoxels = min((_dim.x - gridCoord.x), nVoxels ) * _dim.y * _dim.z;
        _lastAllocatedIndex += outputNVoxels;
    }
}
