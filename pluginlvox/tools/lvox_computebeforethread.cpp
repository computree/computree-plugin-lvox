#include "lvox_computebeforethread.h"
#include "ct_itemdrawable/ct_beam.h"
#include "ct_itemdrawable/tools/gridtools/ct_grid3dwootraversalalgorithm.h"
#include "tools/lvox_distancevisitor.h"
#include "tools/lvox_countvisitor.h"

LVOX_ComputeBeforeThread::LVOX_ComputeBeforeThread(CT_Scanner *scanner,
                                                   CT_Grid3D<int> *outputBeforeGrid,
                                                   CT_Grid3D<double> *outputDeltaBeforeGrid,
                                                   const CT_Scene *scene,
                                                   double intensityThresh,
                                                   bool greaterThan) : QThread()
{
    _scanner = scanner;
    _outputBeforeGrid = outputBeforeGrid;
    _outputDeltaBeforeGrid = outputDeltaBeforeGrid;
    _scene = scene;
    _intensityThresh = intensityThresh;
    _greaterThan = greaterThan;
}

void LVOX_ComputeBeforeThread::run()
{
    const CT_AbstractPointCloud *pointCloud = _scene->getPointCloud();
    const CT_AbstractPointCloudIndex *pointCloudIndex = _scene->getPointCloudIndex();
    quint64 n_points = pointCloudIndex->indexSize();

    QVector3D bot, top;
    _outputBeforeGrid->getMinCoordinates(bot);
    _outputBeforeGrid->getMaxCoordinates(top);

    // Creates visitors
    LVOX_DistanceVisitor distVisitor(_outputDeltaBeforeGrid);
    LVOX_CountVisitor countVisitor(_outputBeforeGrid);
    QList<CT_AbstractGrid3DBeamVisitor*> list;
    list.append(&distVisitor);
    list.append(&countVisitor);

    // Creates traversal algorithm
    CT_Grid3DWooTraversalAlgorithm<int> algo(_outputBeforeGrid, false, list);

    CT_Beam beam(NULL, NULL);
    QVector3D origin, direction;

    for (quint64 i = 0 ; i < n_points; i++)
    {
        const int &index = (*pointCloudIndex)[i];
        const CT_Point &point = (*pointCloud)[index];

        direction.setX(point.x - _scanner->getPosition().x());
        direction.setY(point.y - _scanner->getPosition().y());
        direction.setZ(point.z - _scanner->getPosition().z());

        origin.setX(point.x);
        origin.setY(point.y);
        origin.setZ(point.z);

        // Get the next ray
        beam.setOrigin(origin);
        beam.setDirection(direction);

        if (beam.intersect(bot, top))
        {
            algo.compute(beam);
        }
    }

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

    // Don't forget to calculate min and max in order to visualize it as a colored map
    _outputBeforeGrid->computeMinMax();
    _outputDeltaBeforeGrid->computeMinMax();
}
