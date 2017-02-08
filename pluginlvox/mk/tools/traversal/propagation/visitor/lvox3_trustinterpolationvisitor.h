/**
 * @author Michael Krebs (AMVALOR)
 * @date 25.01.2017
 * @version 1
 */
#ifndef LVOX3_TRUSTINTERPOLATIONVISITOR_H
#define LVOX3_TRUSTINTERPOLATIONVISITOR_H

#include "lvox3_propagationvisitor.h"
#include "mk/tools/lvox3_gridtype.h"

/**
 * @brief Use this visitor to interpolate a grid with a trust (confiance) factor
 */
class LVOX3_TrustInterpolationVisitor : public LVOX3_PropagationVisitor
{
public:
    /**
     * @brief LVOX3_TrustInterpolationVisitor. Between effectiveRayThreshold and endRayThreshold the function will be sin((Nt-Nb)-effectiveRayThreshold)
     * @param outDensityGrid : grid to modify
     * @param effectiveRayThreshold : minimum number of effective ray (Nt-Nb must be > to threshold) to have a trust factor > 0
     * @param endRayThreshold : maximum number of effective ray. If (Nt-Nb) > endRayThreshold the trust factor will be 1
     */
    LVOX3_TrustInterpolationVisitor(const lvox::Grid3Df* outDensityGrid,
                                    const lvox::Grid3Di* inBeforeGrid,
                                    const lvox::Grid3Di* inTheoriticalsGrid,
                                    qint32 effectiveRayThreshold,
                                    qint32 endRayThreshold);

    /**
     * @brief Called when the propagation start
     * @param context : will contains all elements compute by the algorithm  when it will start. Contains
     *                  information of the first cell
     */
    void start(const LVOX3_PropagationVisitorContext& context);

    /**
     * @brief Called when a voxel must be visited
     * @param context : will contains all elements compute by the algorithm  when it will start. Contains
     *                  information of the current cell
     */
    void visit(const LVOX3_PropagationVisitorContext& context);

    /**
     * @brief Called when the propagation was finished
     * @param context : will contains all elements compute by the algorithm  when it will start. Contains
     *                  information of the first cell
     */
    void finish(const LVOX3_PropagationVisitorContext& context);

private:
    lvox::Grid3Df*  m_grid;
    lvox::Grid3Di*  m_beforeGrid;
    lvox::Grid3Di*  m_theoriticalsGrid;
    qint32          m_effectiveRayThreshold;
    qint32          m_endRayThreshold;
    double          m_diffEffectiveRay;
    double          m_denominator;
    double          m_numerator;
};

#endif // LVOX3_TRUSTINTERPOLATIONVISITOR_H
