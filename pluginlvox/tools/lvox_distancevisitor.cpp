#include "lvox_distancevisitor.h"


LVOX_DistanceVisitor::LVOX_DistanceVisitor(CT_Grid3D<double> *grid)
{
  _grid = grid;
}

void LVOX_DistanceVisitor::visit(const size_t &index, const CT_Beam *beam)
{
    QVector3D bot, top, nearInter, farInter;
    bool ok = _grid->getCellCoordinates(index, bot, top);

    if (ok && beam->intersect(bot, top, nearInter, farInter))
    {
        _grid->addValueAtIndex(index, sqrt(pow(nearInter.x()-farInter.x(), 2) + pow(nearInter.y()-farInter.y(), 2) + pow(nearInter.z()-farInter.z(), 2)));
    }
}
