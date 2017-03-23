#include <QtTest>
#include <QDebug>
#include <QString>
#include <QVector>
#include <QFile>

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
    void testPolarCoordinates();
    void testAngleBenchmark();
    void testComputeFieldOfView();
    void testMiniMaxi();
    void testCrossProductSign();

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

    cout << pattern << endl;

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

#include <vector>

QDebug operator<<(QDebug debug, const Vector3d &v)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << '(' << v.x() << ", " << v.y() << ", " << v.z() << ')';

    return debug;
}

class MiniMaxi {
public:
    MiniMaxi() :
        m_mini(std::numeric_limits<double>::max()),
        m_maxi(-std::numeric_limits<double>::max()) {}
    void update(double value) {
        m_mini = std::min(value, m_mini);
        m_maxi = std::max(value, m_maxi);
    }
    double min() const { return m_mini; }
    double max() const { return m_maxi; }
    double len() const { return m_maxi - m_mini; }
private:
    double m_mini;
    double m_maxi;
};

QDebug operator<<(QDebug debug, const MiniMaxi &mm)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << '[' << mm.min() << ", " << mm.max() << ']';

    return debug;
}

class ThetaPhiBounds {
public:
    ThetaPhiBounds(const Vector3d& orig) :
        m_origin(orig) { }
    void extend(const Vector3d& pt) {
        Vector3d v = pt - m_origin;
        double r = v.norm();
        double theta = acos(v.z() / r);
        double phi = atan2(v.y(), v.x());
        m_thetaBounds.update(theta);
        m_phiBounds.update(phi);
    }
    static double phiAngle(Vector3d& v1, Vector3d v2) {
        return atan2(v1.y(), v1.x()) - atan2(v2.y(), v2.x());
    }
    double thetaAngle() {
        //double r = v.norm();
        //double theta = acos(v.z() / r) - acos(...);
        return 0.0;
    }
    MiniMaxi theta() const { return m_thetaBounds; }
    MiniMaxi phi() const { return m_phiBounds; }
private:
    const Vector3d m_origin;
    MiniMaxi m_thetaBounds;
    MiniMaxi m_phiBounds;
};

QDebug operator<<(QDebug debug, const ThetaPhiBounds &b)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << '(' << b.theta() << ", " << b.phi() << ')';

    return debug;
}

ThetaPhiBounds thetaPhiBounds(const Vector3d& orig, const vector<Vector3d>& pts)
{
    ThetaPhiBounds bounds(orig);
    for (uint i = 0; i < pts.size(); i++) {
        bounds.extend(pts[i]);
    }
    qDebug() << "theta bounds: " << qRadiansToDegrees(bounds.theta().min()) << qRadiansToDegrees(bounds.theta().max());
    qDebug() << "phi   bounds: " << qRadiansToDegrees(bounds.phi().min()) << qRadiansToDegrees(bounds.phi().max());
    return bounds;
}

void ScannersTest::testMiniMaxi()
{
    MiniMaxi mm;
    QVERIFY(mm.min() > 0);
    QVERIFY(mm.max() < 0);
    mm.update(42);
    QCOMPARE(mm.min(), 42.);
    QCOMPARE(mm.max(), 42.);
    mm.update(-42);
    QCOMPARE(mm.min(), -42.);
    QCOMPARE(mm.max(), 42.);
}

double absAngle(Vector3d& v1, Vector3d& v2)
{
    Vector3d c = v1.cross(v2);
    Vector3d up(0, 0, 1);
    double sign = up.dot(c);
    double angle = atan2(c.norm(), v1.dot(v2));
    if (sign < 0) {
        angle = -angle;
    }
    return angle;
}

void ScannersTest::testPolarCoordinates()
{
    Vector3d scan(4, 4, 1);
    vector<Vector3d> pts1 = {
        Vector3d(8, 3, 0),
        Vector3d(9, 5, 2),
        Vector3d(9, 7, 3),
    };

    vector<Vector3d> pts2 = {
        Vector3d(5, 5, 0),
        Vector3d(5, 5, 2),
        Vector3d(5, 7, 3),
    };

    vector<Vector3d> pts3 = {
        Vector3d(5, 5, 1),
        Vector3d(3, 5, 1),
        Vector3d(3, 3, 1),
        Vector3d(5, 3, 1),
    };

    qDebug() << thetaPhiBounds(scan, pts1);
    qDebug() << thetaPhiBounds(scan, pts2);
    qDebug() << thetaPhiBounds(scan, pts3);


    Vector3d p1(1, 2, 0);
    Vector3d p2(-1, 2, 0);
    Vector3d p3(-1, -2, 0);
    Vector3d p4(1, -2, 0);

    // angle between two vectors is always in the interval [0, 180]
    qDebug() << qRadiansToDegrees(absAngle(p1, p2));
    qDebug() << qRadiansToDegrees(absAngle(p1, p3));
    qDebug() << qRadiansToDegrees(absAngle(p1, p4));
    qDebug() << qRadiansToDegrees(absAngle(p4, p1));

}

void ScannersTest::testAngleBenchmark()
{
    // About 100ns on Intel i7-4600U
    Vector3d p1(1, 2, 0);
    Vector3d p2(-1, 2, 0);
    QBENCHMARK {
        volatile double a = absAngle(p1, p2);
    }
}

void computeFieldOfView(AlignedBox3d& box, Vector3d& scan)
{
    // get the center of the box
    Vector3d center = box.center();
    Vector3d scanToCenter = center - scan;

    cout << "center: " << center.transpose() << endl;
    cout << "scanToCenter:    " << scanToCenter.transpose() << endl;

    vector<AlignedBox3d::CornerType> corners = {
        AlignedBox3d::BottomLeftFloor,
        AlignedBox3d::BottomRightFloor,
        AlignedBox3d::TopLeftFloor,
        AlignedBox3d::TopRightFloor,
        AlignedBox3d::BottomLeftCeil,
        AlignedBox3d::BottomRightCeil,
        AlignedBox3d::TopLeftCeil,
        AlignedBox3d::TopRightCeil
    };

    MiniMaxi mmPhi;
    for (uint i = 0; i < corners.size(); i++) {
        Vector3d corner = box.corner(corners[i]);
        Vector3d scanToCorner = corner - scan;
        double phiRelative = absAngle(scanToCenter, scanToCorner);
        qDebug() << corner << scanToCorner;
        qDebug() << "phi rel:" << qRadiansToDegrees(phiRelative);
        mmPhi.update(phiRelative);
    }
    Vector3d xAxis(1, 0, 0);
    double rotate = absAngle(xAxis, scanToCenter);
    qDebug() << "phi rel range:"
             << qRadiansToDegrees(mmPhi.min())
             << qRadiansToDegrees(mmPhi.max())
             << qRadiansToDegrees(mmPhi.len())
             << qRadiansToDegrees(rotate);
}

void ScannersTest::testComputeFieldOfView()
{
    /*
     * Create a bouding box of 10 units and calculate the angles from various
     * points arround (think of a rubik's cube). A translation is applied to the
     * box to verify that the algorithm does not assume the box is at the
     * origin.
     *
     *      X  X  X
     *       +---+
     *      X|   |X
     *       +---+
     *      X  X  X
     */

    AlignedBox3d ibox(Vector3d(2, 2, 2), Vector3d(12, 12, 12));
    vector<Vector3d> pos = {
        Vector3d(14, 7, 7),
        Vector3d(7, 14, 7),
    };

    for (uint i = 0; i < pos.size(); i++) {
        computeFieldOfView(ibox, pos[i]);
    }

}

void ScannersTest::testCrossProductSign()
{
    Vector3d v1(1, 0, 0);
    Vector3d v2(0, 1, 0);
    Vector3d v3(0, -1, 0);

    double a1 = absAngle(v1, v2);
    double a2 = absAngle(v1, v3);
    QCOMPARE(a1, M_PI_2);
    QCOMPARE(a2, -M_PI_2);
}

QTEST_APPLESS_MAIN(ScannersTest)

#include "tst_scanners.moc"
