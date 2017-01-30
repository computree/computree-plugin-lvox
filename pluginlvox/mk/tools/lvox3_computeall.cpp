#include "lvox3_computeall.h"

#include <QtConcurrent/QtConcurrent>

LVOX3_ComputeAll::LVOX3_ComputeAll() : LVOX3_Worker()
{
}

LVOX3_ComputeAll::~LVOX3_ComputeAll()
{
    qDeleteAll(m_workers.begin(), m_workers.end());
}

void LVOX3_ComputeAll::addWorker(int startIndex, LVOX3_Worker *worker)
{
    m_workers.insert(startIndex, worker);
}

void LVOX3_ComputeAll::doTheJob()
{
    QList<int> keys = m_workers.uniqueKeys();

    setProgressRange(0, m_workers.values().size()*100);

    m_progressOffset = 0;

    foreach (int key, keys) {
        m_currentWorkers = m_workers.values(key);

        foreach (LVOX3_Worker* worker, m_currentWorkers) {
            connect(this, SIGNAL(cancelRequested()), worker, SLOT(cancel()), Qt::DirectConnection);
            connect(worker, SIGNAL(progressChanged(int)), this, SLOT(progressFromWorkerChanged()), Qt::DirectConnection);
            //worker->compute();
        }

        // start thread....
        QFuture<void> future = QtConcurrent::map(m_currentWorkers, startWorker);
        future.waitForFinished();

        foreach (LVOX3_Worker* worker, m_currentWorkers) {
            disconnect(this, NULL, worker, NULL);
            disconnect(worker, NULL, this, NULL);
        }

        if(mustCancel())
            return;

        m_progressOffset += m_currentWorkers.size()*100;
    }
}

void LVOX3_ComputeAll::startWorker(LVOX3_Worker *worker)
{
    worker->compute();
}

void LVOX3_ComputeAll::progressFromWorkerChanged()
{
    int progress = m_progressOffset;

    foreach (LVOX3_Worker* worker, m_currentWorkers) {
        progress += worker->getProgress();
    }

    setProgress(progress);
}
