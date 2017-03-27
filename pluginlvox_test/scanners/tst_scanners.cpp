#include <QtTest>
#include <QDebug>
#include <QString>
#include <QVector>
#include <QFile>
#include <QTextStream>

#include <memory>
#include <cmath>
#include <iostream>

#include "Eigen/Dense"
#include "Eigen/Geometry"

#include "ct_global/ct_context.h"
#include "ct_defines.h"

#include "ct_itemdrawable/tools/scanner/ct_thetaphishootingpattern.h"
#include "ct_itemdrawable/tools/scanner/ct_shootingpatternfrompointcloud.h"
#include "ct_itemdrawable/tools/scanner/ct_parallelshootingpatternfrompointcloud.h"
#include "ct_cloudindex/registered/abstract/ct_abstractnotmodifiablecloudindexregisteredt.h"
#include "mk/tools/worker/lvox3_computetheoriticals.h"
#include "mk/tools/lvox3_gridtype.h"
#include "mk/tools/lvox3_errorcode.h"
#include "mk/tools/worker/lvox3_computeall.h"
#include "mk/tools/traversal/woo/lvox3_grid3dwootraversalalgorithm.h"

using namespace Eigen;
using namespace std;

/*
 * SRCDIR is defined by the compiler, but Eclipse doesn't
 * know it and complains. Define a value here to make it happy.
 */
#ifndef SRCDIR
#define SRCDIR "."
#endif

QDebug operator<<(QDebug debug, const Vector3d &v)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << '(' << v.x() << ", " << v.y() << ", " << v.z() << ')';

    return debug;
}

class ScannersTest : public QObject
{
    Q_OBJECT

public:
    ScannersTest();

private Q_SLOTS:
    void testTLS();
    void testPointCloudShootingPattern();
    void testParallelShootingPattern();
    void testBoudingBox();
    void testTheoreticalGrid();
    void testThetaPhiShootingPattern();
    void testAlignedBoxes();

private:
    CT_PCIR m_pcir;
};

ScannersTest::ScannersTest()
{
    CT_AbstractUndefinedSizePointCloud* cloud =
            PS_REPOSITORY->createNewUndefinedSizePointCloud();
    cloud->addPoint(CT_Point(Vector3d(1, 2, 3)));
    cloud->addPoint(CT_Point(Vector3d(4, 5, 6)));
    m_pcir = PS_REPOSITORY->registerUndefinedSizePointCloud(cloud);
}

void ScannersTest::testTLS()
{
    Vector3d origin(0, 0, 0);
    Vector3d normal(0, 0, 1);
    double theta = 90.;
    double phi = 90.;
    double res = 1.;
    size_t n = 8100;

    /*
     * Create a stub shooting pattern with 90 degrees of field of view with 1
     * degree of resolution.
     */

    CT_ThetaPhiShootingPattern p(origin, theta, phi, res, res, 0, 0, normal,
                                 false, false);
    QVERIFY2(p.getNumberOfShots() == n, "wrong number of shots");

    for (uint i = 0; i < n - 1; i++) {
        CT_Shot s1 = p.getShotAt(i);
        CT_Shot s2 = p.getShotAt(i + 1);
        QVERIFY2(s1.getOrigin() == origin, "s1 origin must match");
        QVERIFY2(s2.getOrigin() == origin, "s2 origin must match");

        Vector3d cross = s1.getDirection().cross(s2.getDirection());
        QVERIFY2(cross.norm() > 0., "shots are not expected to be parallel");
    }
}


void ScannersTest::testPointCloudShootingPattern()
{
    /*
     * Create a cloud of points for CT_ShootingPatternFromPointCloud
     */

    Vector3d origin(10, 10, 10);
    CT_ShootingPatternFromPointCloud p(origin, m_pcir);
    QVERIFY(p.getNumberOfShots() == 2);
    CT_Shot s1 = p.getShotAt(0);
    CT_Shot s2 = p.getShotAt(1);
    QVERIFY(s1.getOrigin() == origin);
    QVERIFY(s2.getOrigin() == origin);
    Vector3d cross = s1.getDirection().cross(s2.getDirection());
    QVERIFY2(cross.norm() > 0, "shots are not expected to be parallel");
}

void ScannersTest::testParallelShootingPattern()
{
    /*
     * The plane is above the points and rays are downwards (nadir)
     */
    Vector3d origin(0, 0, 10);
    Vector3d direction(0, 0, -1);

    CT_ParallelShootingPatternFromPointCloud p(origin, direction, m_pcir);

    QVERIFY(p.getNumberOfShots() == 2);
    CT_Shot s1 = p.getShotAt(0);
    CT_Shot s2 = p.getShotAt(1);
    Vector3d cross1 = s1.getDirection().cross(direction);
    Vector3d cross2 = s2.getDirection().cross(direction);
    QCOMPARE(cross1.norm(), 0.);
    QCOMPARE(cross2.norm(), 0.);
}

void ScannersTest::testBoudingBox()
{
    AlignedBox3d b1(Vector3d(-1, -2, -3), Vector3d(1, 2, 3));
    AlignedBox3d b2(Vector3d(-3, -2, -1), Vector3d(3, 2, 3));
    b1.extend(b2);
    Vector3d sizes = b1.sizes();
    QVERIFY(sizes == Vector3d(6, 4, 6));
    QVERIFY(b1.center() == Vector3d(0, 0, 0));

    /*
     * Top/Bottom really means Front/Back
     */
    QVERIFY(b1.corner(AlignedBox3d::BottomLeftFloor) == Vector3d(-3, -2, -3));
    QVERIFY(b1.corner(AlignedBox3d::BottomRightFloor) == Vector3d(3, -2, -3));
    QVERIFY(b1.corner(AlignedBox3d::TopLeftFloor) == Vector3d(-3, 2, -3));
    QVERIFY(b1.corner(AlignedBox3d::TopRightFloor) == Vector3d(3, 2, -3));

    QVERIFY(b1.corner(AlignedBox3d::BottomLeftCeil) == Vector3d(-3, -2, 3));
    QVERIFY(b1.corner(AlignedBox3d::BottomRightCeil) == Vector3d(3, -2, 3));
    QVERIFY(b1.corner(AlignedBox3d::TopLeftCeil) == Vector3d(-3, 2, 3));
    QVERIFY(b1.corner(AlignedBox3d::TopRightCeil) == Vector3d(3, 2, 3));
}

class DebugVisitor : public LVOX3_Grid3DVoxelWooVisitor
{
public:
    DebugVisitor() {}
    void visit(const LVOX3_Grid3DVoxelWooVisitorContext& context) {
        qDebug() << "visit: " << context.colLinLevel.x() << context.colLinLevel.y() << context.colLinLevel.z();
    }
};

void ScannersTest::testTheoreticalGrid()
{
    /*
        const Eigen::Vector3d& origin,
                               double hFov, double vFov,
                               double hRes, double vRes,
                               double initTheta, double initPhi,
                               const Eigen::Vector3d& zVector = Eigen::Vector3d(0,0,1),
                               bool clockWise = true,
                               bool radians = false
    */

    double z = 10;
    CT_ThetaPhiShootingPattern pattern(
        Vector3d(5., 1., z),
        90, 180,
        10, 10,
        0., 0.);

    lvox::Grid3Di grid(NULL, NULL,
            10., 10., 10.,
            10UL, 10UL, 10UL,
            1, lvox::Max_Error_Code, 0);
    qDebug() << "grid.nCells()" << grid.nCells();

    Vector3d min, max;
    grid.getBoundingBox(min, max);
    cout << min << "\n" << max << endl;

    QVector<LVOX3_Grid3DVoxelWooVisitor*> visitors;
    visitors.append(new DebugVisitor());
    LVOX3_Grid3DWooTraversalAlgorithm<lvox::Grid3DiType> algo(&grid, true, visitors);

    size_t n = pattern.getNumberOfShots();
    qDebug() << "number of shots" << n;
    for (size_t i = 0; i < n; i++) {
        const CT_Shot& shot = pattern.getShotAt(i);
        algo.compute(shot.getOrigin(), shot.getDirection());
    }

    /*
     * It is mandatory to allocate the object here with "new", the
     * workersManager calls "delete" on all workers in its destructor.
     */
//    LVOX3_ComputeTheoriticals *worker = new LVOX3_ComputeTheoriticals(&pattern, &grid);
//    LVOX3_ComputeAll workersManager;
//    workersManager.addWorker(0, worker);
//    workersManager.compute();

}

double normalizeAngle(double rad)
{
    static const double two_pi = M_PI * 2;
    if (rad > two_pi || rad < 0)
        rad -= two_pi * std::floor(rad / two_pi);
    return rad;
}

void ScannersTest::testThetaPhiShootingPattern()
{
    double val = 0.1;
    QCOMPARE(normalizeAngle(val), val);
    QCOMPARE(normalizeAngle(M_PI * 2), M_PI * 2);
    QCOMPARE(normalizeAngle(val + M_PI), val + M_PI);
    QCOMPARE(normalizeAngle(val + M_PI * 2), val);
    QCOMPARE(normalizeAngle(val + M_PI * 3), val + M_PI);
    QCOMPARE(normalizeAngle(val + M_PI * 4), val);
    QCOMPARE(normalizeAngle(-val), 2 * M_PI - val);
    QCOMPARE(normalizeAngle(-val - M_PI * 3), M_PI - val);

    /* Shooting pattern */
    /*
    CT_ThetaPhiShootingPattern pattern(Vector3d::Zero(),
        90, 180, 10, 10, 90, 0, Vector3d(0, 0, 1), false, true);
    */

    /*
     * Using the mathematical convention: theta is the angle against the X axis
     * and phi is the angle against the Z axis.
     */
    double t0 = normalizeAngle(qDegreesToRadians(0.));
    double t1 = normalizeAngle(qDegreesToRadians(180.));
    double p0 = normalizeAngle(qDegreesToRadians(45.));
    double p1 = normalizeAngle(qDegreesToRadians(190.));
    double hfov = t1 - t0;
    double vfov = p1 - p0;

    qDebug() << t0 << t1 << p0 << p1
             << qRadiansToDegrees(hfov) << qRadiansToDegrees(vfov);

    double rays = 100;
    double dt = hfov / rays;
    double dp = vfov / rays;

    QFile outFile("rays.asc");
    outFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
    QTextStream out(&outFile);
    for (int i = 0; i < rays; i++) {
        double theta = t0 + dt * i;
        for (int j = 0; j < rays; j++) {
            double phi = p0 + dp * j;
            // converting to cartesian
            double x = std::sin(phi) * std::cos(theta);
            double y = std::sin(phi) * std::sin(theta);
            double z = std::cos(phi);
            qDebug() << qRadiansToDegrees(theta) << qRadiansToDegrees(phi)
                     << x << y << z;
            out << x << " " << y << " " << z << "\n";
        }
    }
}

void ScannersTest::testAlignedBoxes()
{
    Vector3d p0(-10, -10, -10);
    Vector3d p1(-5, -5, -5);
    Vector3d p2(10, 10, 10);
    Vector3d p3(20, 20, 20);

    AlignedBox3d temp;
    const AlignedBox3d p0p1(p0, p1);
    const AlignedBox3d p0p2(p0, p2);
    const AlignedBox3d p1p2(p1, p2);
    const AlignedBox3d p1p3(p1, p3);
    const AlignedBox3d p2p3(p2, p3);
    const AlignedBox3d p0p3(p0, p3);

    QVERIFY(p0p1.contains(p1));
    QVERIFY(!p0p1.contains(p2));
    QVERIFY(!p0p1.contains(p3));

    temp = p0p1;
    temp = temp.extend(p2);
    QVERIFY(temp.contains(p1));
    QVERIFY(temp.contains(p2));
    QVERIFY(!temp.contains(p3));

    temp = p0p2.intersection(p1p3);
    QVERIFY(temp.isApprox(p1p2));

    temp = p0p2.merged(p1p3);
    QVERIFY(temp.isApprox(p0p3));

    AlignedBox3d exp(Vector3d(-6, -6, -6), Vector3d(11, 11, 11));
    temp = p1p2;
    Vector3d v1 = p1p2.min().array() - 1;
    Vector3d v2 = p1p2.max().array() + 1;
    temp.extend(v1);
    temp.extend(v2);
    QVERIFY(temp.isApprox(exp));
}

QTEST_APPLESS_MAIN(ScannersTest)

#include "tst_scanners.moc"
