#include <QString>
#include <QtTest>
#include <QDebug>

#include "ct_itemdrawable/ct_grid3d.h"
#include "mk/tools/lvox3_gridtools.h"

class Grid_neighborsTest : public QObject
{
    Q_OBJECT

public:
    Grid_neighborsTest();

private Q_SLOTS:
    void testCase1();
};

Grid_neighborsTest::Grid_neighborsTest()
{
}

void Grid_neighborsTest::testCase1()
{
    CT_Grid3D<int> grid;
    grid.computeMinMax();

    LVOX3_GridTools tools(&grid);
    size_t idx = 10;
    tools.computeGridIndexForColLinLevel(0, 0, 0, idx);

    QVERIFY2(idx == 0, "index");
}

QTEST_APPLESS_MAIN(Grid_neighborsTest)

#include "tst_grid_neighborstest.moc"
