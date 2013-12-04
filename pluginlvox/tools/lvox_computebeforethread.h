#ifndef LVOX_COMPUTEBEFORETHREAD_H
#define LVOX_COMPUTEBEFORETHREAD_H

#include <QThread>
#include "ct_itemdrawable/ct_scanner.h"
#include "ct_itemdrawable/ct_grid3d.h"
#include "ct_itemdrawable/ct_scene.h"
#include "ct_tools/ct_monitoredqthread.h"

class LVOX_ComputeBeforeThread : public CT_MonitoredQThread
{
public:
    LVOX_ComputeBeforeThread(CT_Scanner *scanner,
                             CT_Grid3D<int> *outputBeforeGrid,
                             CT_Grid3D<double> *outputDeltaBeforeGrid,
                             const CT_Scene *scene,
                             bool computeDistance);

    void run();

private:
    CT_Scanner*         _scanner;
    CT_Grid3D<int>*     _outputBeforeGrid;
    CT_Grid3D<double>*  _outputDeltaBeforeGrid;
    const CT_Scene*     _scene;
    bool                _computeDistance;
};

#endif // LVOX_COMPUTEBEFORETHREAD_H
