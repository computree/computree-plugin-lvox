#ifndef LVOX3_MERGEGRIDS_H
#define LVOX3_MERGEGRIDS_H

#include <memory>
#include <QVector>
#include "mk/tools/lvox3_utils.h"

enum VoxelReducerType {
    MaxRDI,
    MaxTrust,
    MaxTrustRatio,
    MaxNi,
    SumRatio,
    ReducerTypeLast
};

struct VoxelData {
    void load(LVOXGridSet &g, size_t idx);
    void commit(LVOXGridSet &g, size_t idx);
    int nt;
    int nb;
    int ni;
    float rd;
};

class VoxelReducerOptions {
public:
    VoxelReducerOptions() :
        reducerType(MaxTrust),
        ignoreVoxelZeroDensity(true),
        effectiveRaysThreshold(10) {}
    VoxelReducerType reducerType;
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
    VoxelReducerType type;
    QString name;
    QString desc;
};

class LVOX3_MergeGrids
{
public:
    LVOX3_MergeGrids();

    static void apply(LVOXGridSet &merged, QVector<LVOXGridSet*> &gs,
                      VoxelReducer &reducer);

    static const VoxelReducerDefinitionStruct VoxelReducerDefinitions[];
};

#endif // LVOX3_MERGEGRIDS_H
