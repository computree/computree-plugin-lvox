#include "lvox3_grid3dpropagationalgorithm.h"

#include "visitor/lvox3_propagationvisitor.h"

#include <QStack>

#define COMPUTECELL(XCOL, XLIN, XLEVEL)   m_gridTools->computeGridIndexForColLinLevel(XCOL, XLIN, XLEVEL, newIndice); \
                                          if (!m_visited.contains(newIndice)) \
                                            stack.push_back(StackVar(newIndice, XCOL, XLIN, XLEVEL));

LVOX3_Grid3DPropagationAlgorithm::LVOX3_Grid3DPropagationAlgorithm(const CT_AbstractGrid3D* grid,
                                                                   const VisitorCollection& list,
                                                                   const double &radius,
                                                                   const size_t &startCellIndex) : m_visitors(list)
{
    m_grid = (CT_AbstractGrid3D*)grid;
    m_gridTools = new LVOX3_GridTools(grid);
    m_radius = radius;
    m_startCellIndex = startCellIndex;
}

LVOX3_Grid3DPropagationAlgorithm::~LVOX3_Grid3DPropagationAlgorithm()
{
    delete m_gridTools;
}

void LVOX3_Grid3DPropagationAlgorithm::startCompute()
{
    startFromCell(m_startCellIndex);
}

void LVOX3_Grid3DPropagationAlgorithm::startFromCell(const size_t& index)
{
    size_t col, lin, level;
    Eigen::Vector3d cellCenter;

    m_gridTools->computeColLinLevelForIndex(index, col, lin, level);
    m_gridTools->computeCellCenterCoordsAtColLinLevel(col, lin, level, cellCenter);

    LVOX3_PropagationVisitorContext context(index, col, lin, level, cellCenter, 0);

    for (int i = 0 ; i < m_visitors.size() ; ++i)
        m_visitors.at(i)->start(context);

    QStack<StackVar> stack;
    stack.push_back(StackVar(index, col, lin, level));

    while(!stack.isEmpty())  {
        StackVar c = stack.pop();
        recursiveComputeCell(c.cellIndex, c.col, c.lin, c.level, cellCenter, stack);
    }

    for (int i = 0 ; i < m_visitors.size() ; ++i)
        m_visitors.at(i)->finish(context);

    m_visited.clear();
}

void LVOX3_Grid3DPropagationAlgorithm::recursiveComputeCell(const size_t &cellIndex,
                                                            const size_t& col,
                                                            const size_t& lin,
                                                            const size_t& level,
                                                            const Eigen::Vector3d &firstCellCenter,
                                                            QStack<StackVar>& stack)
{
    m_visited.insert(cellIndex);

    const size_t maxCol = m_grid->xdim()-1;
    const size_t maxLin = m_grid->ydim()-1;
    const size_t maxLevel = m_grid->zdim()-1;

    Eigen::Vector3d newCellCenter;

    m_gridTools->computeCellCenterCoordsAtColLinLevel(col, lin, level, newCellCenter);

    const double distanceBetweenCells = qAbs((firstCellCenter - newCellCenter).norm());

    if(distanceBetweenCells < m_radius) {

        LVOX3_PropagationVisitorContext context(cellIndex, col, lin, level, newCellCenter, distanceBetweenCells);

        for (int i = 0 ; i < m_visitors.size() ; ++i)
            m_visitors.at(i)->visit(context);

        size_t newIndice;

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
