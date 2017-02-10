/**
 * @author Michael Krebs (AMVALOR)
 * @date 25.01.2017
 * @version 1
 */
#ifndef LVOX3_INTERPOLATEDISTANCE_H
#define LVOX3_INTERPOLATEDISTANCE_H

#include "lvox3_worker.h"
#include "mk/tools/lvox3_gridtype.h"

/**
 * @brief Interpolate the grid density by using cells values that was in a defined radius
 *
 * The formula used is : Sum(IDR / D^p)
 *                       --------------------
 *                       Sum(1 / D^p)
 *
 * "IDR" = value of density of the cell inspected
 * "D" the distance between the cell to change density and the cell inspected in the radius
 * "p" the power value defined
 */
class LVOX3_InterpolateDistance : public LVOX3_Worker
{
    Q_OBJECT

public:
    /**
     * @brief Construct this worker
     * @param originalDensityGrid : the original density grid to get values of density
     * @param outDensityGrid : the density grid that will be modified with new density values
     * @param radius : max radius to search cells
     * @param power : power that will be used in the formula
     */
    LVOX3_InterpolateDistance(const lvox::Grid3Df* originalDensityGrid,
                              lvox::Grid3Df* outDensityGrid,
                              double radius,
                              int power);

protected:
    /**
     * @brief Do the job
     */
    void doTheJob();

private:
    lvox::Grid3Df*      m_originalDensityGrid;
    lvox::Grid3Df*      m_outDensityGrid;
    double              m_radius;
    int                 m_power;
};

#endif // LVOX3_INTERPOLATEDISTANCE_H
