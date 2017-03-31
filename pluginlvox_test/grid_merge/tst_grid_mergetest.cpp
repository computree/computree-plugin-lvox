#include <QString>
#include <QtTest>

#include <Eigen/Dense>
#include <memory>
#include <vector>

#include "ct_cloudindex/registered/abstract/ct_abstractnotmodifiablecloudindexregisteredt.h"
#include "ct_defines.h"
#include "ct_point.h"

#include "ct_itemdrawable/tools/scanner/ct_parallelshootingpatternfrompointcloud.h"
#include "mk/tools/worker/lvox3_computehits.h"
#include "mk/tools/worker/lvox3_computebefore.h"
#include "mk/tools/worker/lvox3_computetheoriticals.h"
#include "mk/tools/lvox3_gridtype.h"
#include "mk/tools/lvox3_gridtools.h"
#include "mk/tools/lvox3_errorcode.h"

using namespace std;
using namespace lvox;
using namespace Eigen;

struct VecPt {
    VecPt() : p(0, 0, 0), n(0) {}
    VecPt(double x, double y, double z, uint n) :
        p(x, y, z), n(n) {}
    Vector3d p;
    uint n;
};

class GridSet {
public:
    GridSet(double min, ulong dim, double res) :
            m_min(min), m_dim(dim), m_res(res) {
        ni.reset(makeCubicGrid<Grid3Di>(min, dim, res));
        nt.reset(makeCubicGrid<Grid3Di>(min, dim, res));
        nb.reset(makeCubicGrid<Grid3Di>(min, dim, res));
        rd.reset(makeCubicGrid<Grid3Df>(min, dim, res));
    }

    static Vector3d mid(Vector3d v, double min, double res) {
        return v.array() * res + min + res / 2;
    }

    void init(vector<VecPt> &pts) {
        CT_AbstractUndefinedSizePointCloud* cloud =
                PS_REPOSITORY->createNewUndefinedSizePointCloud();
        for (const VecPt& pt: pts) {
            for (uint i = 0; i < pt.n; i++) {
                Vector3d v = mid(pt.p, m_min, m_res);
                qDebug() << pt.p.x() << pt.p.y() << pt.p.z() << v.x() << v.y() << v.z();
                cloud->addPoint(CT_Point(v));
            }
        }
        CT_PCIR pcir = PS_REPOSITORY->registerUndefinedSizePointCloud(cloud);
        CT_AbstractPointCloudIndex *apci = dynamic_cast<CT_AbstractPointCloudIndex*>(pcir->abstractCloudIndexT());

        Vector3d dir(1, 0, 0);
        Vector3d min(m_min, m_min, m_min);
        CT_ParallelShootingPatternFromPointCloud sp(min, dir, pcir);

        LVOX3_ComputeHits w1(&sp, apci, ni.get());
        w1.compute();

        LVOX3_ComputeBefore w2(&sp, apci, nb.get());
        w2.compute();

        LVOX3_ComputeTheoriticals w3(&sp, nt.get());
        w3.compute();
    }

    static void dumpGrid(Grid3Di* g) {
        for (uint k = 0; k < g->zdim(); k++) {
            printf("level %d\n", k);
            for (uint j = 0; j < g->ydim(); j++) {
                for (uint i = 0; i < g->xdim(); i++) {
                    printf("%d ", g->valueAtXYZ(i, j, k));
                }
                printf("\n");
            }
        }
    }

    template<class T>
    T *makeCubicGrid(double min, ulong dim, double res) {
        return new T(NULL, NULL, min, min, min, dim, dim, dim,
                     res, Max_Error_Code, 0);
    }

    unique_ptr<Grid3Di> ni;
    unique_ptr<Grid3Di> nt;
    unique_ptr<Grid3Di> nb;
    unique_ptr<Grid3Df> rd;
    double m_min;
    ulong m_dim;
    double m_res;
};

enum MyGridSets {
    GRID_A = 0,
    GRID_B = 1,
    GRID_MAX = 2
};

class Grid_mergeTest : public QObject
{
    Q_OBJECT

public:
    Grid_mergeTest();

private Q_SLOTS:
    void testCase1();
};

Grid_mergeTest::Grid_mergeTest()
{
}

vector<GridSet*> *computeGrids(vector<vector<VecPt>> &pts)
{
    double m = 0;
    ulong d = 3;
    double r = 1;
    vector<GridSet*> *gs = new vector<GridSet*>();
    for (uint i = 0; i < pts.size(); i++) {
        GridSet* set = new GridSet(m, d, r);
        set->init(pts[i]);
        gs->push_back(set);
    }
    return gs;
}

void dumpGridIndexes(Grid3Di* g) {
    LVOX3_GridTools gt(g);
    for (uint k = 0; k < g->zdim(); k++) {
        printf("level %d\n", k);
        for (uint j = 0; j < g->ydim(); j++) {
            for (uint i = 0; i < g->xdim(); i++) {
                size_t idx;
                gt.computeGridIndexForColLinLevel(i, j, k, idx);
                printf("%d ", idx);
            }
            printf("\n");
        }
    }
}
void Grid_mergeTest::testCase1()
{
    Vector3d exp(10.05, 10.15, 10.25);
    Vector3d act = GridSet::mid(Vector3d(0, 1, 2), 10, 0.1);
    QVERIFY(act == exp);

    // make two point clouds
    // GRID_A: ird 1.0
    // GRID_B: ird 0.5
    vector<vector<VecPt>> pts = {
        { VecPt(0, 1, 1, 1) ,
          VecPt(1, 1, 1, 2) },
        { VecPt(1, 1, 1, 1) }
    };

    // compute grids
    vector<GridSet*> *gs = computeGrids(pts);

    //Grid3Di g;
    //g.valueAtXYZ()
    GridSet *a = gs->at(GRID_A);
    GridSet *b = gs->at(GRID_B);
    printf("indexes\n");
    dumpGridIndexes(a->ni.get());

    printf("ni\n");
    GridSet::dumpGrid(a->ni.get());
    GridSet::dumpGrid(b->ni.get());

    QCOMPARE(a->ni->valueAtXYZ(0, 1, 1), 1);
    QCOMPARE(a->ni->valueAtXYZ(1, 1, 1), 2);
    QCOMPARE(a->ni->valueAtXYZ(2, 1, 1), 0);

    QCOMPARE(b->ni->valueAtXYZ(1, 1, 1), 1);
    QCOMPARE(b->ni->valueAtXYZ(1, 2, 1), 0);

    printf("nb\n");
    GridSet::dumpGrid(a->nb.get());
    GridSet::dumpGrid(b->nb.get());
    QCOMPARE(a->nb->valueAtXYZ(0, 1, 1), 0);
    QCOMPARE(a->nb->valueAtXYZ(1, 1, 1), 1);
    QCOMPARE(a->nb->valueAtXYZ(2, 1, 1), 3);
    QCOMPARE(b->nb->valueAtXYZ(2, 1, 1), 1);

    printf("nt\n");
    GridSet::dumpGrid(a->nt.get());
    GridSet::dumpGrid(b->nt.get());

    // merge grids
    // verify result


    // cleanup
    qDeleteAll(gs->begin(), gs->end());
    delete gs;
}

QTEST_APPLESS_MAIN(Grid_mergeTest)

#include "tst_grid_mergetest.moc"
