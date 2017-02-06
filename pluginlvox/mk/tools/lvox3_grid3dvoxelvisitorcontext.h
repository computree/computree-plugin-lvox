/**
 * @author Michael Krebs (AMVALOR)
 * @date 25.01.2017
 * @version 1
 */
#ifndef LVOX3_GRID3DVOXELVISITORCONTEXT_H
#define LVOX3_GRID3DVOXELVISITORCONTEXT_H

#include "Eigen/Core"

struct LVOX3_Grid3DVoxelVisitorContext {
    typedef Eigen::Matrix<size_t, 3, 1> Vector3SizeT;

    LVOX3_Grid3DVoxelVisitorContext(const Eigen::Vector3d& origin,
                                    const Eigen::Vector3d& direction) : rayOrigin(origin),
                                                                        rayDirection(direction) {}

    size_t                          currentVoxelIndex;          /*! voxel index in the grid */
    Vector3SizeT                    colLinLevel;                /*! vector with X, Y, Z = column, line, z level of the current voxel in grid */
    const Eigen::Vector3d&          rayOrigin;                  /*! ray origin */
    const Eigen::Vector3d&          rayDirection;               /*! ray direction */
    Eigen::Vector3d                 nearImpactPointWithGrid;    /*! first impact point of the ray with the grid (bbox of the grid) */
    Eigen::Vector3d                 farImpactPointWithGrid;     /*! second impact point of the ray with the grid (bbox of the grid) */
};

#endif // LVOX3_GRID3DVOXELVISITORCONTEXT_H
