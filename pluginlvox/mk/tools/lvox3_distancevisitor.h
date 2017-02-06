/**
 * @author Michael Krebs (AMVALOR)
 * @date 25.01.2017
 * @version 1
 */
#ifndef LVOX3_DISTANCEVISITOR_H
#define LVOX3_DISTANCEVISITOR_H

#include "lvox3_grid3dvoxelvisitor.h"
#include "lvox3_rayboxintersectionmath.h"
#include "lvox3_gridtools.h"
#include "lvox3_gridtype.h"
#include "lvox3_errorcode.h"

#include "ct_itemdrawable/ct_grid3d.h"

template<typename T>
class LVOX3_DistanceVisitor : public LVOX3_Grid3DVoxelVisitor
{
public:
    LVOX3_DistanceVisitor(const CT_Grid3D<T>* grid,
                          const lvox::MutexCollection* collection = NULL) {
        m_grid = (CT_Grid3D<T>*)grid;
        m_multithreadCollection = (lvox::MutexCollection*)collection;
        m_gridTools = NULL;

        if(grid != NULL)
            m_gridTools = new LVOX3_GridTools(grid);
    }

    ~LVOX3_DistanceVisitor() {
        delete m_gridTools;
    }

    /**
     * @brief Called when a voxel must be visited
     */
    void visit(const LVOX3_Grid3DVoxelVisitorContext& context) {
        Eigen::Vector3d bot, top, nearInter, farInter;
        m_gridTools->computeCellBottomLeftTopRightCornerAtColLinLevel(context.colLinLevel.x(),
                                                                      context.colLinLevel.y(),
                                                                      context.colLinLevel.z(),
                                                                      bot,
                                                                      top);

        if (LVOX3_RayBoxIntersectionMath::getIntersectionOfRay(bot, top, context.rayOrigin, context.rayDirection, nearInter, farInter))
        {
            const double distance = (nearInter - farInter).norm();

            if(m_multithreadCollection != NULL) {
                QMutex* mutex = (*m_multithreadCollection)[context.currentVoxelIndex];
                mutex->lock();
                m_grid->addValueAtIndex(context.currentVoxelIndex, distance);
                mutex->unlock();
            } else {
                m_grid->addValueAtIndex(context.currentVoxelIndex, distance);
            }
        }
    }

private:
    CT_Grid3D<T>*           m_grid;
    LVOX3_GridTools*        m_gridTools;
    lvox::MutexCollection*  m_multithreadCollection;
};

#endif // LVOX3_DISTANCEVISITOR_H
