#include "lvox3_distanceinterpolationvisitor.h"

LVOX3_DistanceInterpolationVisitor::LVOX3_DistanceInterpolationVisitor(const lvox::Grid3Df *densityGrid, const int &power)
{
    m_grid = (lvox::Grid3Df*)densityGrid;
    m_power = power;
}

void LVOX3_DistanceInterpolationVisitor::start(const LVOX3_PropagationVisitorContext &context)
{
    Q_UNUSED(context)

    m_numerator = 0;
    m_denominator = 0;
}

void LVOX3_DistanceInterpolationVisitor::visit(const LVOX3_PropagationVisitorContext &context)
{
    if(context.m_distance > 0) {
        const lvox::Grid3DfType density = m_grid->valueAtIndex(context.m_cellIndex);

        if(density > 0) {
            const double denom = std::pow(context.m_distance, m_power);
            m_numerator += ((double)density)/denom;
            m_denominator += denom;
        }
    }
}

void LVOX3_DistanceInterpolationVisitor::finish(const LVOX3_PropagationVisitorContext &context)
{
    if(m_denominator != 0) {
        m_grid->setValueAtIndex(context.m_cellIndex,  m_numerator/m_denominator);
    }
}
