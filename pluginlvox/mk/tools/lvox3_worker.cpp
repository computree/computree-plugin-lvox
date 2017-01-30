#include "lvox3_worker.h"

#include <QElapsedTimer>
#include <QDebug>

LVOX3_Worker::LVOX3_Worker()
{
    m_progress = 0;
    m_progressMin = 0;
    m_progressMax = 100;
    m_progressRange = 100;
}

bool LVOX3_Worker::mustCancel() const
{
    return m_cancel;
}

int LVOX3_Worker::getProgress() const
{
    return m_progress;
}

int LVOX3_Worker::getProgressRangeMin() const
{
    return m_progressMin;
}

int LVOX3_Worker::getProgressRangeMax() const
{
    return m_progressMax;
}

int LVOX3_Worker::getProgressRange() const
{
    return m_progressRange;
}

void LVOX3_Worker::compute()
{
    QElapsedTimer timer;
    timer.start();

    m_cancel = false;
    setProgress(0);

    doTheJob();

    qDebug() << metaObject()->className() << " elapsed : " << timer.elapsed();
}

void LVOX3_Worker::cancel()
{
    m_cancel = true;

    emit cancelRequested();
}

void LVOX3_Worker::setProgress(int newProgress)
{
    Q_ASSERT(m_progressRange != 0);

    int newProgressAdjusted = ((newProgress + m_progressMin) * 100.0) / m_progressRange;

    if(m_progress != newProgressAdjusted)
    {
        m_progress = newProgressAdjusted;
        emit progressChanged(newProgressAdjusted);
    }
}

void LVOX3_Worker::setProgressRange(int min, int max)
{
    if((m_progressMin != min)
            || (m_progressMax != max))
    {
        m_progressMin = min;
        m_progressMax = max;

        if(m_progressMin > m_progressMax)
            qSwap(m_progressMin, m_progressMax);

        m_progressRange = m_progressMax-m_progressMin;
    }
}
