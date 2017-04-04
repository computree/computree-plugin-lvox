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
#include "mk/tools/worker/lvox3_genericcompute.h"
#include "mk/step/lvox3_stepgenericcomputegrids.h"

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
        ni.reset(makeCubicGrid<Grid3Di>(m_min, m_dim, m_res));
        nt.reset(makeCubicGrid<Grid3Di>(m_min, m_dim, m_res));
        nb.reset(makeCubicGrid<Grid3Di>(m_min, m_dim, m_res));
        rd.reset(makeCubicGrid<Grid3Df>(m_min, m_dim, m_res));
    }

    GridSet(const GridSet& other) :
        GridSet(other.m_min, other.m_dim, other.m_res) {
    }

    static Vector3d mid(Vector3d v, double min, double res) {
        return v.array() * res + min + res / 2;
    }

    static vector<GridSet*> *computeGrids(vector<vector<VecPt>> &pts,
                                   double m = 0, ulong d = 3, double r = 1)
    {
        vector<GridSet*> *gs = new vector<GridSet*>();
        for (uint i = 0; i < pts.size(); i++) {
            GridSet* set = new GridSet(m, d, r);
            set->init(pts[i]);
            gs->push_back(set);
        }
        return gs;
    }

    void init(vector<VecPt> &pts) {
        CT_AbstractUndefinedSizePointCloud* cloud =
                PS_REPOSITORY->createNewUndefinedSizePointCloud();
        for (const VecPt& pt: pts) {
            for (uint i = 0; i < pt.n; i++) {
                Vector3d v = mid(pt.p, m_min, m_res);
                //qDebug() << pt.p.x() << pt.p.y() << pt.p.z() << v.x() << v.y() << v.z();
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

        PredefinedConfiguration conf =
                LVOX3_StepGenericComputeGrids::createLVOXDensityPredefinedConfiguration();
        QList<LVOX3_GenericCompute::Input> workerInputs = {
            { 'a', ni.get() }, { 'b', nt.get() }, { 'c', nb.get() }
        };
        OutGridConfiguration out = conf.output.first();
        LVOX3_GenericCompute w4(workerInputs, out.checks, out.getFormula(), rd.get());
        w4.compute();
    }

    static void dumpGrid(CT_AbstractGrid3D* g) {
        for (uint k = 0; k < g->zdim(); k++) {
            cout << "level " << k << endl;
            for (uint j = 0; j < g->ydim(); j++) {
                for (uint i = 0; i < g->xdim(); i++) {
                    size_t idx;
                    g->index(i, j, k, idx);
                    cout << g->valueAtIndexAsDouble(idx) << " ";
                }
                cout << endl;
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
    void testCreateGrids();
    void testMergeMaxRDI();
    void testMergeMaxTrust();
    void testMergeMaxTrustRatio();
    void testMergeMaxNi();
    void testMergeSumRatio();
    void testGenericGridMerge();
};

Grid_mergeTest::Grid_mergeTest()
{
}

void dumpGridIndexes(Grid3Di* g) {
    LVOX3_GridTools gt(g);
    for (uint k = 0; k < g->zdim(); k++) {
        printf("level %d\n", k);
        for (uint j = 0; j < g->ydim(); j++) {
            for (uint i = 0; i < g->xdim(); i++) {
                size_t idx;
                gt.computeGridIndexForColLinLevel(i, j, k, idx);
                printf("%lu ", idx);
            }
            printf("\n");
        }
    }
}
void Grid_mergeTest::testCreateGrids()
{
    Vector3d exp(10.05, 10.15, 10.25);
    Vector3d act = GridSet::mid(Vector3d(0, 1, 2), 10, 0.1);
    QVERIFY(act == exp);

    // make two point clouds
    // GRID_A: ird 1.0
    // GRID_B: ird 0.5
    vector<vector<VecPt>> pts = {
        { VecPt(0, 1, 1, 10) ,
          VecPt(1, 1, 1, 20) },
        { VecPt(1, 1, 1, 10) }
    };

    // compute grids
    vector<GridSet*> *gs = GridSet::computeGrids(pts);

    //Grid3Di g;
    //g.valueAtXYZ()
    GridSet *a = gs->at(GRID_A);
    GridSet *b = gs->at(GRID_B);
    printf("indexes\n");
    dumpGridIndexes(a->ni.get());

    printf("ni\n");
    GridSet::dumpGrid(a->ni.get());
    GridSet::dumpGrid(b->ni.get());

    QCOMPARE(a->ni->valueAtXYZ(0, 1, 1), 10);
    QCOMPARE(a->ni->valueAtXYZ(1, 1, 1), 20);
    QCOMPARE(a->ni->valueAtXYZ(2, 1, 1), 0);

    QCOMPARE(b->ni->valueAtXYZ(1, 1, 1), 10);
    QCOMPARE(b->ni->valueAtXYZ(1, 2, 1), 0);

    printf("nb\n");
    GridSet::dumpGrid(a->nb.get());
    GridSet::dumpGrid(b->nb.get());
    QCOMPARE(a->nb->valueAtXYZ(0, 1, 1), 0);
    QCOMPARE(a->nb->valueAtXYZ(1, 1, 1), 10);
    QCOMPARE(a->nb->valueAtXYZ(2, 1, 1), 30);
    QCOMPARE(b->nb->valueAtXYZ(2, 1, 1), 10);

    printf("nt\n");
    GridSet::dumpGrid(a->nt.get());
    GridSet::dumpGrid(b->nt.get());
    QCOMPARE(a->nt->valueAtXYZ(0, 1, 1), 30);
    QCOMPARE(a->nt->valueAtXYZ(1, 1, 1), 30);
    QCOMPARE(a->nt->valueAtXYZ(2, 1, 1), 30);
    QCOMPARE(b->nt->valueAtXYZ(0, 1, 1), 10);
    QCOMPARE(b->nt->valueAtXYZ(1, 1, 1), 10);
    QCOMPARE(b->nt->valueAtXYZ(2, 1, 1), 10);

    printf("rd\n");
    GridSet::dumpGrid(a->rd.get());
    GridSet::dumpGrid(b->rd.get());

    // cleanup
    qDeleteAll(gs->begin(), gs->end());
    delete gs;
}

struct VoxelData {
    void load(GridSet *g, size_t idx) {
        nt = g->nt->valueAtIndex(idx);
        nb = g->nb->valueAtIndex(idx);
        ni = g->ni->valueAtIndex(idx);
        rd = g->rd->valueAtIndex(idx);
    }
    void commit(GridSet *g, size_t idx) {
        g->nt->setValueAtIndex(idx, nt);
        g->nb->setValueAtIndex(idx, nb);
        g->ni->setValueAtIndex(idx, ni);
        g->rd->setValueAtIndex(idx, rd);
    }
    int nt;
    int nb;
    int ni;
    float rd;
};

class VoxelReducerOptions {
public:
    VoxelReducerOptions() :
        ignoreVoxelZeroDensity(true),
        effectiveRaysThreshold(10) {}
    bool ignoreVoxelZeroDensity;
    int effectiveRaysThreshold;
};

class VoxelReducer {
public:
    VoxelReducer() {}
    VoxelReducer(VoxelReducerOptions& opts) :
        m_opts(opts) {}
    virtual ~VoxelReducer() {}
    virtual void init(VoxelData &data) {
        m_data = data;
    }
    virtual void join(const VoxelData &rhs) = 0;
    virtual VoxelData& value() { return m_data; }
    VoxelData m_data;
    VoxelReducerOptions m_opts;
};

class VoxelReducerMaxRDI : public VoxelReducer {
public:
    VoxelReducerMaxRDI() : VoxelReducer() {}
    void join(const VoxelData &rhs) {
        if (rhs.rd > m_data.rd) {
            m_data = rhs;
        }
    }
};

class VoxelReducerMaxTrust : public VoxelReducer {
public:
    VoxelReducerMaxTrust() : VoxelReducer() {}
    void join(const VoxelData &rhs) {
        if ((rhs.nt - rhs.nb) > (m_data.nt - m_data.nb)) {
            m_data = rhs;
        }
    }
};

class VoxelReducerMaxTrustRatio : public VoxelReducer {
public:
    VoxelReducerMaxTrustRatio() : VoxelReducer() {}
    void join(const VoxelData &rhs) {
        if (m_opts.ignoreVoxelZeroDensity && rhs.rd <= 0) {
            return;
        }
        float ratioSelf = 0;
        float ratioOther = 0;
        if (rhs.nt > 0) {
            ratioOther = (float)(rhs.nt - rhs.nb) / rhs.nt;
        }
        if (m_data.nt > 0) {
            ratioSelf = (float)(m_data.nt - m_data.nb) / m_data.nt;
        }
        if (ratioOther > ratioSelf) {
            m_data = rhs;
        }
    }
};

class VoxelReducerMaxNi : public VoxelReducer {
public:
    VoxelReducerMaxNi() : VoxelReducer() {}
    void join(const VoxelData &rhs) {
        if (rhs.ni > m_data.ni) {
            m_data = rhs;
        }
    }
};

class VoxelReducerSumRatio : public VoxelReducer {
public:
    VoxelReducerSumRatio() : VoxelReducer() {}
    void join(const VoxelData &rhs) {
        if ((m_data.nt - m_data.nb) > m_opts.effectiveRaysThreshold) {
            m_data.nt += rhs.nt;
            m_data.ni += rhs.ni;
            m_data.nb += rhs.nb;
        }
    }
    VoxelData& value() {
        m_data.rd = 0;
        if ((m_data.nt - m_data.nb) > 0) {
            m_data.rd = (float)m_data.ni / (m_data.nt - m_data.nb);
        }
        return m_data;
    }
};

void Grid_mergeTest::testMergeMaxRDI()
{
    VoxelData d1 = { 100, 0, 50, 0.5 };
    VoxelData d2 = {  20, 0, 20, 1.0 };

    VoxelReducerMaxRDI reducer;
    reducer.init(d1);
    reducer.join(d2);
    VoxelData& d3 = reducer.value();
    QCOMPARE(d3.rd, (float)1.0);
}

void Grid_mergeTest::testMergeMaxTrust()
{
    VoxelData d1 = { 100, 0, 50, 0.5 };
    VoxelData d2 = {  20, 0, 20, 1.0 };

    VoxelReducerMaxTrustRatio reducer;
    reducer.init(d1);
    reducer.join(d2);
    VoxelData& d3 = reducer.value();
    QCOMPARE(d3.rd, (float)0.5);
}

void Grid_mergeTest::testMergeMaxTrustRatio()
{
    VoxelData d1 = { 100, 50, 50, 0.5 };
    VoxelData d2 = {  20,  0, 20, 1.0 };

    VoxelReducerMaxTrustRatio reducer;
    reducer.init(d1);
    reducer.join(d2);
    VoxelData& d3 = reducer.value();
    QCOMPARE(d3.rd, (float)1.0);
}

void Grid_mergeTest::testMergeMaxNi()
{
    VoxelData d1 = { 100, 50, 50, 0.5 };
    VoxelData d2 = {  20,  0, 20, 1.0 };

    VoxelReducerMaxNi reducer;
    reducer.init(d1);
    reducer.join(d2);
    VoxelData& d3 = reducer.value();
    QCOMPARE(d3.rd, (float)0.5);
}

void Grid_mergeTest::testMergeSumRatio()
{
    VoxelData d1 = { 100, 50, 50, 0.5 };
    VoxelData d2 = {  40, 10, 20, 1.0 };

    // (50+20)/((100-50)+(40-10)) = 70/80
    // sum(ni)/sum(nt-nb)

    VoxelReducerSumRatio reducer;
    reducer.init(d1);
    reducer.join(d2);
    VoxelData& d3 = reducer.value();
    QCOMPARE(d3.rd, (float)70/80);
}

void mergeGeneric(GridSet* merged, vector<GridSet*> *gs, VoxelReducer &reducer)
{
    if (gs->empty())
        return;

    GridSet *a = gs->at(0);
    size_t n = a->rd->nCells();
    VoxelData item, rhs;
    for (size_t idx = 0; idx < n; idx++) {
        // default values
        item.load(a, idx);
        reducer.init(item);
        for (uint gi = 1; gi < gs->size(); gi++) {
            GridSet *set = gs->at(gi);
            rhs.load(set, idx);
            reducer.join(rhs);
        }
        reducer.value().commit(merged, idx);
    }
}

void Grid_mergeTest::testGenericGridMerge()
{
    vector<vector<VecPt>> pts = {
        { VecPt(0, 1, 1, 30),  // 0.5 ratio
          VecPt(1, 1, 1, 30)  },
        { VecPt(0, 1, 1, 200), // 0.4 ratio, but greater confidence
          VecPt(1, 1, 1, 300) }
    };

    // compute grids
    vector<GridSet*> *gs = GridSet::computeGrids(pts);

    // merge grids
    GridSet *a = gs->at(GRID_A);
    GridSet *b = gs->at(GRID_B);
    GridSet::dumpGrid(a->rd.get());
    GridSet::dumpGrid(b->rd.get());

    {
        printf("VoxelReducerMaxRDI\n");
        VoxelReducerMaxRDI reducer;
        GridSet *merged = new GridSet(*a);
        mergeGeneric(merged, gs, reducer);
        GridSet::dumpGrid(merged->rd.get());
        QCOMPARE(merged->rd->valueAtXYZ(0, 1, 1), (float)0.5);
        delete merged;
    }

    {
        printf("VoxelReducerMaxTrust\n");
        VoxelReducerMaxTrust reducer;
        unique_ptr<GridSet> merged(new GridSet(*a));
        mergeGeneric(merged.get(), gs, reducer);
        GridSet::dumpGrid(merged->rd.get());
        QCOMPARE(merged->rd->valueAtXYZ(0, 1, 1), (float)0.4);
    }

    {
        printf("VoxelReducerMaxTrustRatio\n");
        VoxelReducerMaxTrustRatio reducer;
        unique_ptr<GridSet> merged(new GridSet(*a));
        mergeGeneric(merged.get(), gs, reducer);
        GridSet::dumpGrid(merged->rd.get());
        QCOMPARE(merged->rd->valueAtXYZ(0, 1, 1), (float)0.5);
    }

    {
        printf("VoxelReducerMaxNi\n");
        VoxelReducerMaxNi reducer;
        unique_ptr<GridSet> merged(new GridSet(*a));
        mergeGeneric(merged.get(), gs, reducer);
        GridSet::dumpGrid(merged->rd.get());
        QCOMPARE(merged->rd->valueAtXYZ(0, 1, 1), (float)0.4);
    }

    {
        printf("VoxelReducerSumRatio\n");
        VoxelReducerSumRatio reducer;
        unique_ptr<GridSet> merged(new GridSet(*a));
        mergeGeneric(merged.get(), gs, reducer);
        GridSet::dumpGrid(merged->rd.get());
        QCOMPARE(merged->rd->valueAtXYZ(0, 1, 1), (float)230/560);
    }
}

QTEST_APPLESS_MAIN(Grid_mergeTest)

#include "tst_grid_mergetest.moc"
