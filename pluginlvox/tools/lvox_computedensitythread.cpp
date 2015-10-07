#include "lvox_computedensitythread.h"

LVOX_ComputeDensityThread::LVOX_ComputeDensityThread(CT_Grid3D<float> *densityGrid,
                                                     CT_Grid3D<int> *hitsGrid,
                                                     CT_Grid3D<int> *theoriticalGrid,
                                                     CT_Grid3D<int> *beforeGrid,
                                                     int effectiveRayThresh) : CT_MonitoredQThread()
{
    _densityGrid = densityGrid;
    _hitsGrid = hitsGrid;
    _theoriticalGrid = theoriticalGrid;
    _beforeGrid = beforeGrid;
    _effectiveRayThresh = effectiveRayThresh;
}

void LVOX_ComputeDensityThread::run()
{
    qDebug() << "Début de LVOX_ComputeDensityThread";
    size_t nbVoxels = _densityGrid->nCells();

    // For each voxel
    for ( size_t i = 0 ; i < nbVoxels ; i++ )
    {
        // Compute the density index
        // Avoid division by 0
        if ( (_theoriticalGrid->valueAtIndex(i) - _beforeGrid->valueAtIndex(i)) == 0 )
        {
            _densityGrid->setValueAtIndex(i, -1);
        }
        // If there is an error (nb > nt)
        else if ( (_theoriticalGrid->valueAtIndex(i) - _beforeGrid->valueAtIndex(i)) < 0 )
        {
            _densityGrid->setValueAtIndex(i, -2);
        }
        // If there is not enough information
        else if ( (_theoriticalGrid->valueAtIndex(i) - _beforeGrid->valueAtIndex(i)) < _effectiveRayThresh )
        {
            _densityGrid->setValueAtIndex(i, -3);
        // Excessive Ni
        } else if (_hitsGrid->valueAtIndex(i) > (_theoriticalGrid->valueAtIndex(i) - _beforeGrid->valueAtIndex(i)))
        {
            _densityGrid->setValueAtIndex(i, 1);
        // Normal case
        }else
        {
            _densityGrid->setValueAtIndex(i, (float)(_hitsGrid->valueAtIndex(i)) / (float)(_theoriticalGrid->valueAtIndex(i) - _beforeGrid->valueAtIndex(i) ) );
        }


   }

    // Don't forget to calculate min and max in order to visualize it as a colored map
    _densityGrid->computeMinMax();
    qDebug() << "Fin de LVOX_ComputeDensityThread";
}

