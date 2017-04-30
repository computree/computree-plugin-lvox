#ifndef LVOX3_MERGEGRIDS_H
#define LVOX3_MERGEGRIDS_H

#include <memory>
#include <QVector>
#include "mk/tools/lvox3_utils.h"
#include "mk/tools/lvox3_factory.hpp"

#define LVOX_REDUCER_RDI "MaxRDI"
#define LVOX_REDUCER_TRUST "MaxTrust"
#define LVOX_REDUCER_TRUST_RATIO "MaxTrustRatio"
#define LVOX_REDUCER_HITS "MaxNi"
#define LVOX_REDUCER_SUM_RATIO "SumRatio"

struct VoxelData {
    void load(LVOXGridSet *g, size_t idx);
    void commit(LVOXGridSet *g, size_t idx);
    int nt;
    int nb;
    int ni;
    float rd;
};

class VoxelReducerOptions {
public:
    VoxelReducerOptions() :
        reducerLabel(),
        ignoreVoxelZeroDensity(true),
        effectiveRaysThreshold(10) {}
    QString reducerLabel;
    bool ignoreVoxelZeroDensity;
    int effectiveRaysThreshold;
};

class VoxelReducer {
public:
    VoxelReducer();
    VoxelReducer(VoxelReducerOptions& opts);
    virtual ~VoxelReducer();
    virtual void init(VoxelData &data);
    virtual void join(const VoxelData &rhs) = 0;
    virtual VoxelData& value();
    VoxelData m_data;
    VoxelReducerOptions m_opts;
};

class VoxelReducerMaxRDI : public VoxelReducer {
public:
    void join(const VoxelData &rhs);
};

class VoxelReducerMaxTrust : public VoxelReducer {
public:
    void join(const VoxelData &rhs);
};

class VoxelReducerMaxTrustRatio : public VoxelReducer {
public:
    void join(const VoxelData &rhs);
};

class VoxelReducerMaxNi : public VoxelReducer {
public:
    void join(const VoxelData &rhs);
};

class VoxelReducerSumRatio : public VoxelReducer {
public:
    void join(const VoxelData &rhs);
    VoxelData& value();
};

struct VoxelReducerDefinitionStruct {
    QString label;
    QString name;
    QString desc;
};

#include <functional>
typedef std::function<bool (const size_t &)> ProgressMonitor;

// TODO: LVOX3_MergeGrids should be converted to singleton

class LVOX3_MergeGrids
{
public:
    LVOX3_MergeGrids();

    static void apply(LVOXGridSet *merged, QVector<LVOXGridSet*> *gs,
                      VoxelReducer *reducer);

    static void apply(LVOXGridSet *merged, QVector<LVOXGridSet*> *gs,
                      VoxelReducer *reducer, ProgressMonitor monitor);

    static const QVector<VoxelReducerDefinitionStruct> getReducerList();

    std::unique_ptr<VoxelReducer> makeReducer(QString &label);

private:
    Factory<VoxelReducer> f;
    static const QVector<VoxelReducerDefinitionStruct> m_voxelReducerDefinitions;
};

#endif // LVOX3_MERGEGRIDS_H
