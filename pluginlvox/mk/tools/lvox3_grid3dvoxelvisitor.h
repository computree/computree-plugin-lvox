/**
 * @author Michael Krebs (AMVALOR)
 * @date 25.01.2017
 * @version 1
 */
#ifndef LVOX3_GRID3DVOXELVISITOR_H
#define LVOX3_GRID3DVOXELVISITOR_H

#include "lvox3_grid3dvoxelvisitorcontext.h"

class LVOX3_Grid3DVoxelVisitor
{
public:
    virtual ~LVOX3_Grid3DVoxelVisitor() {}

    /**
     * @brief Called when a voxel must be visited
     */
    virtual void visit(const LVOX3_Grid3DVoxelVisitorContext& context) = 0;
};

#endif // LVOX3_GRID3DVOXELVISITOR_H
