#include "lvox3_interpolatedistance.h"

#include "mk/tools/lvox3_gridtools.h"
#include "mk/tools/lvox3_errorcode.h"

#include <QSet>

#define COMPUTECELL(XCOL, XLIN, XLEVEL)   ne.thisClass->m_gridTools->computeGridIndexForColLinLevel(XCOL, XLIN, XLEVEL, newNe.indice); \
                                          recursiveComputeCell(newNe);

LVOX3_InterpolateDistance::LVOX3_InterpolateDistance(const lvox::Grid3Df *originalDensityGrid,
                                                     lvox::Grid3Df *outDensityGrid,
                                                     double radius,
                                                     int power)
{
    m_originalDensityGrid = (lvox::Grid3Df*)originalDensityGrid;
    m_outDensityGrid = outDensityGrid;
    m_radius = radius;
    m_power = power;
    m_gridTools = new LVOX3_GridTools(m_originalDensityGrid);
}

LVOX3_InterpolateDistance::~LVOX3_InterpolateDistance()
{
    delete m_gridTools;
}

void LVOX3_InterpolateDistance::computeCell(const NecessaryElements& ne)
{
    size_t col, lin, level;
    QSet<size_t> visited;

    double numerator;
    double denominator;

    NecessaryElements newNe(ne);
    newNe.numerator = &numerator;
    newNe.denominator = &denominator;
    newNe.visited = &visited;

    ne.thisClass->m_gridTools->computeColLinLevelForIndex(ne.indice, col, lin, level);
    ne.thisClass->m_gridTools->computeCellCenterCoordsAtColLinLevel(col, lin, level, newNe.cellCenter);
    recursiveComputeCell(newNe);

    ne.thisClass->m_outDensityGrid->setValueAtIndex(ne.indice, numerator/denominator);
}

void LVOX3_InterpolateDistance::doTheJob()
{
    const size_t& nCells = m_originalDensityGrid->nCells();

    for(size_t i=0; i<nCells; ++i) {
        const lvox::Grid3DfType density = m_originalDensityGrid->valueAtIndex(i);

        if(density == lvox::Nt_Minus_Nb_Inferior_Threshold) {
            computeCell(NecessaryElements(this, i));
        }
    }
}

void LVOX3_InterpolateDistance::recursiveComputeCell(const NecessaryElements& ne)
{
    if (ne.visited->contains(ne.indice))
        return;

    ne.visited->insert(ne.indice);

    const size_t maxCol = ne.thisClass->m_originalDensityGrid->xdim()-1;
    const size_t maxLin = ne.thisClass->m_originalDensityGrid->ydim()-1;
    const size_t maxLevel = ne.thisClass->m_originalDensityGrid->zdim()-1;

    NecessaryElements newNe(ne);
    size_t col, lin, level;

    ne.thisClass->m_gridTools->computeCellCenterCoordsAtColLinLevel(col, lin, level, newNe.cellCenter);

    double dist = qAbs((ne.cellCenter - newNe.cellCenter).norm());

    if(dist < ne.thisClass->m_radius) {

        if(dist > 0) {
            const lvox::Grid3DfType density = ne.thisClass->m_originalDensityGrid->valueAtIndex(ne.indice);

            if(density > 0) {
                const double denom = std::pow(dist, ne.thisClass->m_power);
                (*ne.numerator) += ((double)density)/denom;
                (*ne.denominator) += denom;
            }
        }

        newNe.cellCenter = ne.cellCenter;

        if(col > 0) {
            COMPUTECELL(col-1, lin, level);
        }

        if(col < maxCol) {
            COMPUTECELL(col+1, lin, level);
        }

        if(lin > 0) {
            COMPUTECELL(col, lin-1, level);
        }

        if(lin < maxLin) {
            COMPUTECELL(col, lin+1, level);
        }

        if(level > 0) {
            COMPUTECELL(col, lin, level-1);
        }

        if(level < maxLevel) {
            COMPUTECELL(col, lin, level+1);
        }
    }
}
