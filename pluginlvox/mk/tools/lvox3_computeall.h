#ifndef LVOX3_COMPUTEALL_H
#define LVOX3_COMPUTEALL_H

#include "lvox3_worker.h"

#include <QMultiMap>

/**
 * @brief Use this class to manage multiple worker
 */
class LVOX3_ComputeAll : public LVOX3_Worker
{
    Q_OBJECT

public:
    LVOX3_ComputeAll();
    ~LVOX3_ComputeAll();

    /**
     * @brief Add a worker to manage
     * @param startIndex : worker with the same index will be started at the same time. And worker
     *                     from index+1 was not started before all workers on current index was finished successfully.
     * @param worker : worker to add
     */
    void addWorker(int startIndex, LVOX3_Worker* worker);

protected:
    /**
     * @brief Do the job
     */
    void doTheJob();

private:
    QMultiMap<int, LVOX3_Worker*>   m_workers;
    QList<LVOX3_Worker*>            m_currentWorkers;
    int                             m_progressOffset;

    /**
     * @brief Called by QtConcurrent to start the worker
     */
    static void startWorker(LVOX3_Worker* worker);

private slots:
    /**
     * @brief Call when the progression of a worker changed
     */
    void progressFromWorkerChanged();
};

#endif // LVOX3_COMPUTEALL_H
