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

using namespace Eigen;

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

QTEST_APPLESS_MAIN(ScannersTest)

#include "tst_scanners.moc"
