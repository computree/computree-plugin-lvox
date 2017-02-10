#include <QString>
#include <QtTest>
#include <QDebug>
#include <QVector>
#include <QScopedPointer>

#include "ct_itemdrawable/ct_grid3d.h"
#include "mk/tools/lvox3_gridtools.h"
#include "mk/tools/lvox3_gridtype.h"
#include "mk/tools/traversal/propagation/lvox3_grid3dpropagationalgorithm.h"
#include "mk/tools/traversal/propagation/visitor/lvox3_propagationvisitor.h"
#include "mk/tools/worker/lvox3_interpolatedistance.h"
#include "mk/tools/lvox3_errorcode.h"

class Grid_neighborsTest : public QObject
{
    Q_OBJECT

public:
    Grid_neighborsTest();

private Q_SLOTS:
    void testGridVisitor();
    void testGridVisitor_data();
    void testDistanceInterpolation();
    void testDistanceInterpolation_data();
};

Grid_neighborsTest::Grid_neighborsTest()
{
}

class TestVisitor : public LVOX3_PropagationVisitor {
public:
    TestVisitor() : m_count(0) { }
    void start(const LVOX3_PropagationVisitorContext& context) { }
    void visit(const LVOX3_PropagationVisitorContext& context) { m_count++; }
    void finish(const LVOX3_PropagationVisitorContext& context) { }
    int m_count;
};

void Grid_neighborsTest::testGridVisitor_data()
{
    QTest::addColumn<double>("radius");
    QTest::addColumn<int>("count");

    QTest::newRow("small radius") << 0.5 << 1;
    QTest::newRow("exact radius") << 1.0 << 1;
    QTest::newRow("first neighbors") << 1.1 << 7;
    QTest::newRow("second neighbors") << 1.5 << 19;
    QTest::newRow("third neighbors") << 1.8 << 27;
    QTest::newRow("all") << 100.0 << 1000;
}

static lvox::Grid3Df* makeCubeGrid(size_t dim, double res, float val = 0.0f)
{
    return new lvox::Grid3Df(nullptr, nullptr, 0, 0, 0, dim, dim, dim, res, 0, val);
}

/*
 * Create a cube grid with 1m voxel. The visitor counts the number of
 * neighbors visited within the radius. The number should match the
 * geometry of the grid.
 */
void Grid_neighborsTest::testGridVisitor()
{
    QFETCH(double, radius);
    QFETCH(int, count);

    size_t dim = 10;
    double res = 1;
    QScopedPointer<lvox::Grid3Df> grid(makeCubeGrid(dim, res));
    LVOX3_GridTools tool(grid.data());
    TestVisitor v;
    QVector<LVOX3_PropagationVisitor*> visitors;
    visitors.append(&v);

    LVOX3_Grid3DPropagationAlgorithm traverse(grid.data(), visitors, radius);

    size_t idx;
    tool.computeGridIndexForColLinLevel(4, 4, 4, idx);
    traverse.startFromCell(idx);

    QCOMPARE(v.m_count, count);
}

void Grid_neighborsTest::testDistanceInterpolation_data()
{
    QTest::addColumn<double>("radius");
    QTest::addColumn<int>("power");
    QTest::addColumn<float>("epsilon");
    QTest::addColumn<float>("exp");
    QTest::addColumn<float>("initValue");
    QTest::addColumn<float>("spotValue");
    QTest::addColumn<size_t>("X");
    QTest::addColumn<size_t>("Y");
    QTest::addColumn<size_t>("Z");

    const float eps1 = 1e-3f;
    const float eps2 = 0.0f;

	/*
	 * The average of a uniform value is neutral.
	 */
    QTest::newRow("trivial1") << 1.1 << 0 << eps1 << 0.5f << 0.5f << 0.5f << 0UL << 0UL << 0UL;
    QTest::newRow("trivial2") << 1.1 << 2 << eps1 << 0.5f << 0.5f << 0.5f << 0UL << 0UL << 0UL;
    QTest::newRow("trivial3") << 1.8 << 2 << eps1 << 0.5f << 0.5f << 0.5f << 0UL << 0UL << 0UL;


    /*
     * All voxels are zero, except one neighbor.
     *
     * There are 3 types of neighbor (think of the center of a rubik's cube)
     *  - 6 voxels touching a surface, distance sqrt(1)
     *  - 12 voxels touching an edge, distance sqrt(2)
     *  - 8 voxels sharing in the corners, distance sqrt(3)
     *
     *  Below, we detail the weight for each voxel type, where one of them has
     *  a greater density.
     *
     */
    const float a = 0.1;
    const float b = 0.5;
    const float num1 = 1*b/1.0 + 5*a/1.0 + 12*a/2.0 + 8*a/3.0;
    const float num2 = 1*b/2.0 + 6*a/1.0 + 11*a/2.0 + 8*a/3.0;
    const float num3 = 1*b/3.0 + 6*a/1.0 + 12*a/2.0 + 7*a/3.0;
    const float den1 = 6*1.0 + 12*1.0/2.0 + 8*1.0/3.0;

    QTest::newRow("diag1") << 1.8 << 2 << eps1 << num1/den1 << a << b << 5UL << 4UL << 4UL;
    QTest::newRow("diag2") << 1.8 << 2 << eps1 << num2/den1 << a << b << 5UL << 5UL << 4UL;
    QTest::newRow("diag3") << 1.8 << 2 << eps1 << num3/den1 << a << b << 5UL << 5UL << 5UL;


    /*
     * Verify correct behavior of epsilon threshold.
     */
    const float zero = 0;
    const float num4 = 1*b/1.0;
    const float den2 = 1*1.0;

    QTest::newRow("consider zeros") << 1.8 << 2 << eps2 << num4/den1 << zero << b << 5UL << 4UL << 4UL;
    QTest::newRow("ignore zeros")   << 1.8 << 2 << eps1 << num4/den2 << zero << b << 5UL << 4UL << 4UL;
}

/*
 * Interpolate one nodata cell with known neighbor values.
 */
void Grid_neighborsTest::testDistanceInterpolation()
{
    QFETCH(double, radius);
    QFETCH(int, power);
    QFETCH(float, epsilon);
    QFETCH(float, exp);
    QFETCH(float, initValue);
    QFETCH(float, spotValue);
    QFETCH(size_t, X);
    QFETCH(size_t, Y);
    QFETCH(size_t, Z);

    size_t dim = 10;
    double res = 1.0;
    QScopedPointer<lvox::Grid3Df> ingrid(makeCubeGrid(dim, res, initValue));
    QScopedPointer<lvox::Grid3Df> outgrid(makeCubeGrid(dim, res));

    ingrid.data()->setValue(X, Y, Z, spotValue);

    LVOX3_InterpolateDistance interpolator(ingrid.data(), outgrid.data(), radius, power, epsilon);

    /*
     * Define one cell as nodata
     */
    size_t idx;
    outgrid->index(4, 4, 4, idx);
    ingrid->setValueAtIndex(idx, lvox::Nt_Minus_Nb_Inferior_Threshold);
    interpolator.compute();

    float act = outgrid->valueAtIndex(idx);
    QCOMPARE(act, exp);
}

QTEST_APPLESS_MAIN(Grid_neighborsTest)

#include "tst_grid_neighborstest.moc"
