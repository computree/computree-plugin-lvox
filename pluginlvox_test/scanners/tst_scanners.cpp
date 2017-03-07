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
    void testProjection();
};

ScannersTest::ScannersTest()
{
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

	std::unique_ptr<CT_ShootingPattern> p(new CT_ThetaPhiShootingPattern(origin, theta, phi, res, res, 0, 0, normal, false, false));
	QVERIFY2(p->getNumberOfShots() == n, "wrong number of shots");

	for (uint i = 0; i < n - 1; i++) {
		CT_Shot s1 = p->getShotAt(i);
		CT_Shot s2 = p->getShotAt(i + 1);
		Vector3d cross = s1.getDirection().cross(s2.getDirection());
		QVERIFY2(cross.norm() > 0., "shots are not expected to be parallel");
	}
}


void ScannersTest::testPointCloudShootingPattern()
{
	/*
	 * Create a cloud of points for CT_ShootingPatternFromPointCloud
	 */

	CT_AbstractUndefinedSizePointCloud* cloud = PS_REPOSITORY->createNewUndefinedSizePointCloud();
	cloud->addPoint(CT_Point(Vector3d(1, 2, 3)));
	cloud->addPoint(CT_Point(Vector3d(4, 5, 6)));
	CT_PCIR pcir = PS_REPOSITORY->registerUndefinedSizePointCloud(cloud);

	Vector3d origin(10, 10, 10);
	CT_ShootingPattern *p = new CT_ShootingPatternFromPointCloud(origin, pcir);
	QVERIFY(p->getNumberOfShots() == 2);
	CT_Shot s1 = p->getShotAt(0);
	CT_Shot s2 = p->getShotAt(1);
	Vector3d cross = s1.getDirection().cross(s2.getDirection());
	QVERIFY2(cross.norm() > 0, "shots are not expected to be parallel");
}

void ScannersTest::testProjection()
{
	Vector3d origin(10, 10, 10);
	Vector3d direction(1, 1, 0);
	Vector3d u(1, 1, 0);
	Vector3d v(0, 1, 0);
	Vector3d w(0, 0, 1);

	direction.normalize();

	Hyperplane<double, 3> plane(direction, origin);
	Vector3d pu = plane.projection(u);
	Vector3d pv = plane.projection(v);

	Quaterniond q = Quaterniond::FromTwoVectors(w, direction);
	Translation3d t(origin);
	Affine3d trans = q * t;
	Vector3d ur = q * u;

	std::cout << ur << std::endl;
	std::cout << ur.dot(direction) << std::endl;


	//std::cout << p3 << std::endl;
	//std::cout << plane.signedDistance(p3) << std::endl;

	/*
	Vector3d a(1, 0, 0);
	a.normalize();
	Vector3d b(1.5, 2.5, 0);
	std::cout << a << std::endl;
	std::cout << b << std::endl;
	std::cout << a.dot(b) << std::endl;
	std::cout << b.dot(a) << std::endl;
	*/
}

QTEST_APPLESS_MAIN(ScannersTest)

#include "tst_scanners.moc"
