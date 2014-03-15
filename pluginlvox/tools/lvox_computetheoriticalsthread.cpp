#include "lvox_computetheoriticalsthread.h"
#include "qdebug.h"
#include "ct_itemdrawable/ct_beam.h"
#include "ct_itemdrawable/tools/gridtools/ct_grid3dwootraversalalgorithm.h"
#include "tools/lvox_distancevisitor.h"
#include "tools/lvox_countvisitor.h"

LVOX_ComputeTheoriticalsThread::LVOX_ComputeTheoriticalsThread(const CT_Scanner *scanner,
                                                               CT_Grid3D<int> *outputTheoriticalGrid,
                                                               CT_Grid3D<double> *outputDeltaTheoriticalGrid,
                                                               bool computeDistance) : CT_MonitoredQThread()
{
    _scanner = scanner;
    _outputTheoriticalGrid = outputTheoriticalGrid;
    _outputDeltaTheoriticalGrid = outputDeltaTheoriticalGrid;
    _computeDistance = computeDistance;
}

void LVOX_ComputeTheoriticalsThread::run()
{
    qDebug() << "Début de LVOX_ComputeTheoriticalsThread / ScanId=" << _scanner->getScanID();
    QVector3D bot, top;
    _outputTheoriticalGrid->getMinCoordinates(bot);
    _outputTheoriticalGrid->getMaxCoordinates(top);

    // Creates the ray traversal algorithm

    int nHorizontalRays = _scanner->getNHRays();
    int nVerticalRays = _scanner->getNVRays();

    // Creates visitors
    QList<CT_AbstractGrid3DBeamVisitor*> list;

    LVOX_CountVisitor countVisitor(_outputTheoriticalGrid);
    list.append(&countVisitor);

    if (_computeDistance)
    {
        LVOX_DistanceVisitor distVisitor(_outputDeltaTheoriticalGrid);
        list.append(&distVisitor);
    }

    // Creates traversal algorithm
    CT_Grid3DWooTraversalAlgorithm algo(_outputTheoriticalGrid, true, list);

    CT_Beam beam(NULL, NULL);

    int progressStep = nHorizontalRays / 20;

    // For all theoritical rays of the scanner
    for ( int i = 0 ; i < nHorizontalRays ; i++ )
    {
        for ( int j = 0 ; j < nVerticalRays ; j++ )
        {
            // Get the next ray
            _scanner->beam(i,j, beam);

            if (beam.intersect(bot, top))
            {
                algo.compute(beam);
            }
        }

        if (i % progressStep == 0)
        {
            _progress = 100*i/nHorizontalRays;
            emit progressChanged();
        }
    }

    // Don't forget to calculate min and max in order to visualize it as a colored map
    _outputTheoriticalGrid->computeMinMax();

    if (_computeDistance)
    {
        // To get the mean distance we have to divide in each voxel the sum of distances by the number of hits
        for (size_t i = 0 ; i < _outputTheoriticalGrid->nCells() ; i++ )
        {
            if ( _outputTheoriticalGrid->valueAtIndex(i) == 0 )
            {
                _outputDeltaTheoriticalGrid->setValueAtIndex(i,-1);
            } else
            {
                _outputDeltaTheoriticalGrid->setValueAtIndex(i, _outputDeltaTheoriticalGrid->valueAtIndex(i)/_outputTheoriticalGrid->valueAtIndex(i));
            }
        }
        _outputDeltaTheoriticalGrid->computeMinMax();
    }

    _progress = 100;
    emit progressChanged();
    qDebug() << "Fin de LVOX_ComputeTheoriticalsThread / ScanId=" << _scanner->getScanID();
}
