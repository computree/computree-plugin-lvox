#include "lvox_distancevisitor.h"


LVOX_DistanceVisitor::LVOX_DistanceVisitor(CT_Grid3D<double> *grid)
{
  _grid = grid;
}

void LVOX_DistanceVisitor::visit(const int &index, const CT_Beam *beam)
{
    QVector3D bot, top, near, far;
    bool ok = _grid->getCellCoordinates(index, bot, top);

    if (ok && beam->intersect(bot, top, near, far))
    {
        _grid->addValueAtIndex(index, sqrt(pow(near.x()-far.x(), 2) + pow(near.y()-far.y(), 2) + pow(near.z()-far.z(), 2)));
    }
}
