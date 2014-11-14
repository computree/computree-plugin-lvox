#include "lvox_computebeforethread.h"
#include "ct_itemdrawable/ct_beam.h"
#include "ct_itemdrawable/tools/gridtools/ct_grid3dwootraversalalgorithm.h"
#include "tools/lvox_distancevisitor.h"
#include "tools/lvox_countvisitor.h"
#include "ct_pointcloudindex/abstract/ct_abstractpointcloudindex.h"

LVOX_ComputeBeforeThread::LVOX_ComputeBeforeThread(const CT_Scanner *scanner,
                                                   CT_Grid3D<int> *outputBeforeGrid,
                                                   CT_Grid3D<double> *outputDeltaBeforeGrid,
                                                   const CT_Scene *scene,
                                                   bool computeDistance) : CT_MonitoredQThread()
{
    _scanner = scanner;
    _outputBeforeGrid = outputBeforeGrid;
    _outputDeltaBeforeGrid = outputDeltaBeforeGrid;
    _scene = scene;
    _computeDistance = computeDistance;
}

void LVOX_ComputeBeforeThread::run()
{
    qDebug() << "Début de LVOX_ComputeBeforeThread / ScanId=" << _scanner->getScanID();
    const CT_AbstractPointCloudIndex *pointCloudIndex = _scene->getPointCloudIndex();
    size_t n_points = pointCloudIndex->size();

    QVector3D bot, top;
    _outputBeforeGrid->getMinCoordinates(bot);
    _outputBeforeGrid->getMaxCoordinates(top);

    // Creates visitors
    QList<CT_AbstractGrid3DBeamVisitor*> list;

    LVOX_CountVisitor countVisitor(_outputBeforeGrid);
    list.append(&countVisitor);

    if (_computeDistance)
    {
        LVOX_DistanceVisitor distVisitor(_outputDeltaBeforeGrid);
        list.append(&distVisitor);
    }

    // Creates traversal algorithm
    CT_Grid3DWooTraversalAlgorithm algo(_outputBeforeGrid, false, list);

    CT_Beam beam(NULL, NULL);
    QVector3D origin, direction;

    size_t progressStep = n_points / 20;

    for (size_t i = 0 ; i < n_points; i++)
    {
        size_t index;
        const CT_Point &point = pointCloudIndex->constTAt(i, index);

        direction.setX(point(CT_Point::X) - _scanner->getPosition().x());
        direction.setY(point(CT_Point::Y) - _scanner->getPosition().y());
        direction.setZ(point(CT_Point::Z) - _scanner->getPosition().z());

        origin.setX(point(CT_Point::X));
        origin.setY(point(CT_Point::Y));
        origin.setZ(point(CT_Point::Z));

        // Get the next ray
        beam.setOrigin(origin);
        beam.setDirection(direction);

        if (beam.intersect(bot, top))
        {
            algo.compute(beam);
        }

        if (i % progressStep == 0)
        {
            _progress = 100*i/n_points;
            emit progressChanged();
        }
    }

    // Don't forget to calculate min and max in order to visualize it as a colored map
    _outputBeforeGrid->computeMinMax();

    if (_computeDistance)
    {
        // To get the mean distance we have to divide in each voxel the sum of distances by the number of hits
        for (int i = 0 ; i < _outputBeforeGrid->nCells() ; i++ )
        {
            if ( _outputBeforeGrid->valueAtIndex(i) == 0 )
            {
                _outputDeltaBeforeGrid->setValueAtIndex(i,-1);
            } else
            {
                _outputDeltaBeforeGrid->setValueAtIndex(i, _outputDeltaBeforeGrid->valueAtIndex(i)/(double)_outputBeforeGrid->valueAtIndex(i));
            }
        }
        _outputDeltaBeforeGrid->computeMinMax();
    }

    _progress = 100;
    emit progressChanged();
    qDebug() << "Fin de LVOX_ComputeBeforeThread / ScanId=" << _scanner->getScanID();
}
