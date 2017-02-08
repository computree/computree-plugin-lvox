#include "lvox3_trustinterpolationvisitor.h"

LVOX3_TrustInterpolationVisitor::LVOX3_TrustInterpolationVisitor(const lvox::Grid3Df* outDensityGrid,
                                                                 const lvox::Grid3Di* inBeforeGrid,
                                                                 const lvox::Grid3Di* inTheoriticalsGrid,
                                                                 qint32 effectiveRayThreshold,
                                                                 qint32 endRayThreshold)
{
    m_grid = (lvox::Grid3Df*)outDensityGrid;
    m_beforeGrid = (lvox::Grid3Di*)inBeforeGrid;
    m_theoriticalsGrid = (lvox::Grid3Di*)inTheoriticalsGrid;
    m_effectiveRayThreshold = effectiveRayThreshold;
    m_endRayThreshold = endRayThreshold;
    m_diffEffectiveRay = m_endRayThreshold-m_effectiveRayThreshold;
}

void LVOX3_TrustInterpolationVisitor::start(const LVOX3_PropagationVisitorContext &context)
{
    Q_UNUSED(context)

    m_numerator = 0;
    m_denominator = 0;
}

void LVOX3_TrustInterpolationVisitor::visit(const LVOX3_PropagationVisitorContext &context)
{
    if(context.m_distance > 0) {
        const lvox::Grid3DfType density = m_grid->valueAtIndex(context.m_cellIndex);

        if(density > 0) {
            const qint32 Nt_minus_Nb = m_theoriticalsGrid->valueAtIndex(context.m_cellIndex) - m_beforeGrid->valueAtIndex(context.m_cellIndex);

            double trustFactor = 0;

            if(Nt_minus_Nb >= m_effectiveRayThreshold) {
                if(Nt_minus_Nb < m_endRayThreshold) {
                    trustFactor = sin((Nt_minus_Nb-m_effectiveRayThreshold)/m_diffEffectiveRay);
                } else {
                    trustFactor = 1;
                }
            }

            m_numerator += ((double)density)*trustFactor;
            m_denominator += trustFactor;
        }
    }
}

void LVOX3_TrustInterpolationVisitor::finish(const LVOX3_PropagationVisitorContext &context)
{
    if(m_denominator != 0) {
        m_grid->setValueAtIndex(context.m_cellIndex,  m_numerator/m_denominator);
    }
}
