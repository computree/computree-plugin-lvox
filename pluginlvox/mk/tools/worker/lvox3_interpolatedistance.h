/**
 * @author Michael Krebs (AMVALOR)
 * @date 25.01.2017
 * @version 1
 */
#ifndef LVOX3_INTERPOLATEDISTANCE_H
#define LVOX3_INTERPOLATEDISTANCE_H

#include "lvox3_worker.h"
#include "mk/tools/lvox3_gridtype.h"

class LVOX3_GridTools;

/**
 * @brief Interpolate the grid density by using cells values that was in a defined radius
 *
 * The formula used is : Sum(IDR / ((D^p)+1))
 *                       --------------------
 *                       Sum((1 / ((D^p)+1))
 *
 * "IDR" = value of density of the cell inspected
 * "D" the distance between the cell to change density and the cell inspected in the radius
 * "p" the power value defined
 */
class LVOX3_InterpolateDistance : public LVOX3_Worker
{
    Q_OBJECT

public:
    struct NecessaryElements {
        NecessaryElements() {
            numerator = NULL;
            denominator = NULL;
            thisClass = NULL;
            indice = 0;
            visited = NULL;
        }

        NecessaryElements(LVOX3_InterpolateDistance* tc, size_t i) {
            numerator = NULL;
            denominator = NULL;
            thisClass = tc;
            indice = i;
            visited = NULL;
        }

        NecessaryElements(const NecessaryElements& other) : numerator(other.numerator), denominator(other.denominator) {
            thisClass = other.thisClass;
            indice = other.indice;
            visited = other.visited;
        }

        NecessaryElements(double& num, double& denom) : numerator(&num), denominator(&denom) {
            thisClass = NULL;
            indice = 0;
        }

        LVOX3_InterpolateDistance* thisClass;
        Eigen::Vector3d cellCenter;
        size_t indice;
        double* numerator;
        double* denominator;
        QSet<size_t>* visited;
    };

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
    ~LVOX3_InterpolateDistance();

    static void computeCell(const NecessaryElements &ne);
protected:
    /**
     * @brief Do the job
     */
    void doTheJob();

private:
    LVOX3_GridTools*    m_gridTools;
    lvox::Grid3Df*      m_originalDensityGrid;
    lvox::Grid3Df*      m_outDensityGrid;
    double              m_radius;
    int                 m_power;

    static void recursiveComputeCell(const NecessaryElements &ne);
};

#endif // LVOX3_INTERPOLATEDISTANCE_H
