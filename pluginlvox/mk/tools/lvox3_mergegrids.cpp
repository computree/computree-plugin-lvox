#include "lvox3_mergegrids.h"
#include <QVector>

const QVector<VoxelReducerDefinitionStruct>
LVOX3_MergeGrids::m_voxelReducerDefinitions = {
    { LVOX_REDUCER_RDI,         "Max density",      "max(rdi)"              },
    { LVOX_REDUCER_TRUST,       "Max trust",        "max(nt - nb)"          },
    { LVOX_REDUCER_TRUST_RATIO, "Max trust ratio",  "max((nt - nb)/nt)"     },
    { LVOX_REDUCER_HITS,        "Max hits",         "max(ni)"               },
    { LVOX_REDUCER_SUM_RATIO,   "Sum ratio",        "sum(ni)/sum(nt - nb)"  },
};

LVOX3_MergeGrids::LVOX3_MergeGrids()
{
    f.registerType<VoxelReducerMaxRDI>(LVOX_REDUCER_RDI);
    f.registerType<VoxelReducerMaxTrust>(LVOX_REDUCER_TRUST);
    f.registerType<VoxelReducerMaxTrustRatio>(LVOX_REDUCER_TRUST_RATIO);
    f.registerType<VoxelReducerMaxNi>(LVOX_REDUCER_HITS);
    f.registerType<VoxelReducerSumRatio>(LVOX_REDUCER_SUM_RATIO);
}

void LVOX3_MergeGrids::apply(LVOXGridSet *merged,
                             QVector<LVOXGridSet*> *gs,
                             VoxelReducer *reducer)
{
    apply(merged, gs, reducer,
        [](const size_t& i) -> bool {
            Q_UNUSED(i);
            return false;
        });
}

void LVOX3_MergeGrids::apply(LVOXGridSet *merged,
                             QVector<LVOXGridSet*> *gs,
                             VoxelReducer *reducer,
                             ProgressMonitor monitor)
{
    if (gs->empty())
        return;

    LVOXGridSet *a = gs->first();
    size_t n = a->rd->nCells();
    VoxelData item, rhs;
    for (size_t idx = 0; idx < n; idx++) {
        // default values
        item.load(a, idx);
        reducer->init(item);
        for (int gi = 1; gi < gs->size(); gi++) {
            LVOXGridSet *set = gs->at(gi);
            rhs.load(set, idx);
            reducer->join(rhs);
        }
        reducer->value().commit(merged, idx);

        if ((idx % 100) == 0) { // rate limit the update
            if (monitor(idx)) {
                break;
            }
        }
    }
}

const QVector<VoxelReducerDefinitionStruct> LVOX3_MergeGrids::getReducerList()
{
    return m_voxelReducerDefinitions;
}

std::unique_ptr<VoxelReducer> LVOX3_MergeGrids::makeReducer(QString &label)
{
    return std::unique_ptr<VoxelReducer>(f.create(label));
}

void VoxelData::load(LVOXGridSet *g, size_t idx) {
    nt = g->nt->valueAtIndex(idx);
    nb = g->nb->valueAtIndex(idx);
    ni = g->ni->valueAtIndex(idx);
    rd = g->rd->valueAtIndex(idx);
}

void VoxelData::commit(LVOXGridSet *g, size_t idx) {
    g->nt->setValueAtIndex(idx, nt);
    g->nb->setValueAtIndex(idx, nb);
    g->ni->setValueAtIndex(idx, ni);
    g->rd->setValueAtIndex(idx, rd);
}

VoxelReducer::VoxelReducer() {}

VoxelReducer::VoxelReducer(VoxelReducerOptions &opts) :
    m_opts(opts) {}

VoxelReducer::~VoxelReducer() {}

void VoxelReducer::init(VoxelData &data) {
    m_data = data;
}

VoxelData &VoxelReducer::value() { return m_data; }

void VoxelReducerMaxRDI::join(const VoxelData &rhs) {
    if (rhs.rd > m_data.rd) {
        m_data = rhs;
    }
}

void VoxelReducerMaxTrust::join(const VoxelData &rhs) {
    if ((rhs.nt - rhs.nb) > (m_data.nt - m_data.nb)) {
        m_data = rhs;
    }
}

void VoxelReducerMaxTrustRatio::join(const VoxelData &rhs) {
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

void VoxelReducerMaxNi::join(const VoxelData &rhs) {
    if (rhs.ni > m_data.ni) {
        m_data = rhs;
    }
}

void VoxelReducerSumRatio::join(const VoxelData &rhs) {
    if ((m_data.nt - m_data.nb) > m_opts.effectiveRaysThreshold) {
        m_data.nt += rhs.nt;
        m_data.ni += rhs.ni;
        m_data.nb += rhs.nb;
    }
}

VoxelData &VoxelReducerSumRatio::value() {
    m_data.rd = 0;
    if ((m_data.nt - m_data.nb) > 0) {
        m_data.rd = (float)m_data.ni / (m_data.nt - m_data.nb);
    }
    return m_data;
}
