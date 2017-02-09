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
	QTest::addColumn<float>("value");

	QTest::newRow("trivial1") << 1.1 << 0 << 1.0f;
	QTest::newRow("trivial2") << 1.1 << 2 << 1.0f;
	QTest::newRow("trivial3") << 1.5 << 2 << 1.0f;
}

/*
 * Interpolate one nodata cell with known neighbor values.
 */
void Grid_neighborsTest::testDistanceInterpolation()
{
	QFETCH(double, radius);
	QFETCH(int, power);
	QFETCH(float, value);

	size_t dim = 10;
	double res = 1;
	QScopedPointer<lvox::Grid3Df> ingrid(makeCubeGrid(dim, res, 1.0f));
	QScopedPointer<lvox::Grid3Df> outgrid(makeCubeGrid(dim, res));

	LVOX3_InterpolateDistance interpolator(ingrid.data(), outgrid.data(), radius, power);

	/*
	 * Define one cell as nodata
	 */
	LVOX3_GridTools tool(outgrid.data());
	size_t idx;
	tool.computeGridIndexForColLinLevel(4, 4, 4, idx);
	ingrid->setValueAtIndex(idx, lvox::Nt_Minus_Nb_Inferior_Threshold);
	interpolator.compute();

	QCOMPARE(outgrid->valueAtIndex(idx), value);
}

QTEST_APPLESS_MAIN(Grid_neighborsTest)

#include "tst_grid_neighborstest.moc"
