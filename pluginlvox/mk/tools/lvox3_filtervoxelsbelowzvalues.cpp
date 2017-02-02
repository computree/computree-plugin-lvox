#include "lvox3_filtervoxelsbelowzvalues.h"

#include "lvox3_gridtools.h"
#include "lvox3_errorcode.h"

LVOX3_FilterVoxelsBelowZValues::LVOX3_FilterVoxelsBelowZValues(const QList<CT_AbstractGrid3D *> &grids,
                                                               const CT_AbstractImage2D *zValues) : LVOX3_Worker()
{
    m_grids = grids.toVector();
    m_zValues = (CT_AbstractImage2D*)zValues;
}

void LVOX3_FilterVoxelsBelowZValues::doTheJob()
{
    int nGrids = m_grids.size();

    if(nGrids == 0)
        return;

    CT_AbstractGrid3D* refGrid = m_grids[0];

    size_t nCells = refGrid->nCells();

    if(nCells == 0)
        return;

    Eigen::Vector3d cellTopCoordinate;
    LVOX3_GridTools gridTools(refGrid);

    double NAValue = m_zValues->NAAsDouble();
    size_t index;
    int j;

    for(size_t i=0; i<nCells; ++i) {
        gridTools.computeCellTopMiddleCoordsAtIndex(i, cellTopCoordinate);

        m_zValues->indexAtCoords(cellTopCoordinate.x(), cellTopCoordinate.y(), index);
        const double minZValue = m_zValues->valueAtIndexAsDouble(index); // TODO : bug in image 2D => return not Nan but quiet_nan !!!

        if(minZValue != NAValue) {
            if(cellTopCoordinate.z() < minZValue) {
                for(j=0; j<nGrids; ++j)
                    m_grids[j]->setValueAtIndexFromDouble(i, lvox::Below_MNT);
            }
        }
    }
}
