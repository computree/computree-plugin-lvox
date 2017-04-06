#include "lvox3_mergegridsworker.h"

#include <QDebug>

LVOX3_MergeGridsWorker::LVOX3_MergeGridsWorker(LVOXGridSet *merged,
                    QVector<LVOXGridSet *> *gs, VoxelReducer *reducer) :
    m_merged(merged), m_gs(gs), m_reducer(reducer)
{
}

void LVOX3_MergeGridsWorker::doTheJob()
{
    qDebug() << "LVOX3_MergeGridWorker::doTheJob()";

    size_t work = m_merged->rd->nCells();
    setProgressRange(0, work);
    LVOX3_MergeGrids::apply(m_merged, m_gs, m_reducer,
        [&](const size_t &i) {
            this->setProgress(i);
            return this->mustCancel();
        }
    );
    setProgress(work);
}
