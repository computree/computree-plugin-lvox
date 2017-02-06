/**
 * @author Michael Krebs (AMVALOR)
 * @date 25.01.2017
 * @version 1
 */
#ifndef LVOX3_GRIDTOOLS_H
#define LVOX3_GRIDTOOLS_H

#include "Eigen/Core"
#include "ct_itemdrawable/abstract/ct_abstractgrid3d.h"

class LVOX3_GridTools {
public:
    LVOX3_GridTools(const CT_AbstractGrid3D* grid) {
        grid->getMinCoordinates(m_gridBBOXMin);
        m_gridDimX = grid->xdim();
        m_gridDimXMultDimY = m_gridDimX * grid->ydim();
        m_gridResolution = grid->resolution();
        m_gridResolutionDiv2 = m_gridResolution/2.0;;
    }

    inline void computeGridIndexForPoint(const Eigen::Vector3d& point,
                                         size_t& indice) {
        const size_t col = computeColLinLevel(m_gridBBOXMin.x(), m_gridResolution, point.x());
        const size_t lin = computeColLinLevel(m_gridBBOXMin.y(), m_gridResolution, point.y());
        const size_t level = computeColLinLevel(m_gridBBOXMin.z(), m_gridResolution, point.z());

        indice = level*m_gridDimXMultDimY + lin*m_gridDimX + col;
    }

    inline void computeGridIndexForPoint(const Eigen::Vector3d& point,
                                         size_t& col,
                                         size_t& lin,
                                         size_t& level,
                                         size_t& indice) {
        col = computeColLinLevel(m_gridBBOXMin.x(), m_gridResolution, point.x());
        lin = computeColLinLevel(m_gridBBOXMin.y(), m_gridResolution, point.y());
        level = computeColLinLevel(m_gridBBOXMin.z(), m_gridResolution, point.z());

        indice = level*m_gridDimXMultDimY + lin*m_gridDimX + col;
    }

    inline void computeGridIndexForColLinLevel(const size_t& col,
                                               const size_t& lin,
                                               const size_t& level,
                                               size_t& indice) {
        indice = level*m_gridDimXMultDimY + lin*m_gridDimX + col;
    }

    inline void computeCellBottomLeftCornerAtPoint(const Eigen::Vector3d& point,
                                                   Eigen::Vector3d& bottomLeftCorner) {

        computeCellBottomLeftCornerAtColLinLevel(computeColLinLevel(m_gridBBOXMin.x(), m_gridResolution, point.x()),
                                                 computeColLinLevel(m_gridBBOXMin.y(), m_gridResolution, point.y()),
                                                 computeColLinLevel(m_gridBBOXMin.z(), m_gridResolution, point.z()),
                                                 bottomLeftCorner);
    }

    inline void computeCellBottomLeftCornerAtColLinLevel(const size_t& col,
                                                         const size_t& lin,
                                                         const size_t& level,
                                                         Eigen::Vector3d& bottomLeftCorner) {
        bottomLeftCorner.x() = m_gridBBOXMin.x() + col * m_gridResolution;
        bottomLeftCorner.y() = m_gridBBOXMin.y() + lin * m_gridResolution;
        bottomLeftCorner.z() = m_gridBBOXMin.z() + level * m_gridResolution;
    }

    inline void computeCellBottomLeftTopRightCornerAtColLinLevel(const size_t& col,
                                                                 const size_t& lin,
                                                                 const size_t& level,
                                                                 Eigen::Vector3d& bottomLeftCorner,
                                                                 Eigen::Vector3d& topRightCorner) {

        computeCellBottomLeftCornerAtColLinLevel(col, lin, level, bottomLeftCorner);
        topRightCorner = bottomLeftCorner.array() + m_gridResolution;
    }

    inline void computeCellTopMiddleCoordsAtIndex(const size_t& index,
                                                  Eigen::Vector3d& coords) {
        size_t col = 0, lin = 0, level = 0;

        if(index >= m_gridDimXMultDimY)
            level = index/m_gridDimXMultDimY;

        if(index >= m_gridDimX)
            lin = (index - (level*m_gridDimXMultDimY))/m_gridDimX;

        col = index - (level*m_gridDimXMultDimY) - (lin*m_gridDimX);

        computeCellBottomLeftCornerAtColLinLevel(col, lin, level, coords);

        coords.x() += m_gridResolutionDiv2;
        coords.y() += m_gridResolutionDiv2;
        coords.z() += m_gridResolution;
    }

    static inline size_t computeColLinLevel(const double& min,
                                            const double& gridResolution,
                                            const double& val) {
        return (size_t) std::floor((val - min) / gridResolution);
    }

private:
    Eigen::Vector3d m_gridBBOXMin;
    double          m_gridDimX;
    double          m_gridDimXMultDimY;
    double          m_gridResolution;
    double          m_gridResolutionDiv2;
};

#endif // LVOX3_GRIDTOOLS_H
