#ifndef LVOX3_COUNTVISITOR_H
#define LVOX3_COUNTVISITOR_H

#include "lvox3_grid3dvoxelvisitor.h"
#include "lvox3_gridtype.h"

#include "ct_itemdrawable/ct_grid3d.h"

template<typename T>
class LVOX3_CountVisitor : public LVOX3_Grid3DVoxelVisitor
{
public:
    LVOX3_CountVisitor(const CT_Grid3D<T>* grid,
                       const lvox::MutexCollection* collection = NULL) {
        m_grid = (CT_Grid3D<T>*)grid;
        m_multithreadCollection = (lvox::MutexCollection*)collection;
    }

    /**
     * @brief Called when a voxel must be visited
     */
    void visit(const LVOX3_Grid3DVoxelVisitorContext& context) {
        if(m_multithreadCollection != NULL) {
            QMutex* mutex = (*m_multithreadCollection)[context.currentVoxelIndex];
            mutex->lock();
            m_grid->addValueAtIndex(context.currentVoxelIndex, 1);
            mutex->unlock();
        } else {
            m_grid->addValueAtIndex(context.currentVoxelIndex, 1);
        }
    }

private:
    CT_Grid3D<T>*           m_grid;
    lvox::MutexCollection*  m_multithreadCollection;
};

#endif // LVOX3_COUNTVISITOR_H
