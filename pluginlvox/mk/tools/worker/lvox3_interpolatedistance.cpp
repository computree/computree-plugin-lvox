#include "lvox3_interpolatedistance.h"

#include "mk/tools/lvox3_errorcode.h"

#include "mk/tools/traversal/propagation/lvox3_grid3dpropagationalgorithm.h"
#include "mk/tools/traversal/propagation/visitor/lvox3_distanceinterpolationvisitor.h"

LVOX3_InterpolateDistance::LVOX3_InterpolateDistance(const lvox::Grid3Df *originalDensityGrid,
                                                     lvox::Grid3Df *outDensityGrid,
                                                     double radius,
                                                     int power)
{
    m_originalDensityGrid = (lvox::Grid3Df*)originalDensityGrid;
    m_outDensityGrid = outDensityGrid;
    m_radius = radius;
    m_power = power;
}

void LVOX3_InterpolateDistance::doTheJob()
{
    LVOX3_DistanceInterpolationVisitor visitor(m_originalDensityGrid,
                                               m_outDensityGrid,
                                               m_power);

    QVector<LVOX3_PropagationVisitor*> l;
    l.append(&visitor);

    LVOX3_Grid3DPropagationAlgorithm algo(m_originalDensityGrid,
                                          l,
                                          m_radius);

    const size_t& nCells = m_originalDensityGrid->nCells();

    for(size_t i=0; i<nCells; ++i) {
        const lvox::Grid3DfType density = m_originalDensityGrid->valueAtIndex(i);

        if(lvox::NoDataCode::isNoData(density)) {
            algo.startFromCell(i);
        }
    }
}
