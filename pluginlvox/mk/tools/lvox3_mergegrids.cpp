#include "lvox3_mergegrids.h"

#include <QVector>

const VoxelReducerDefinitionStruct
LVOX3_MergeGrids::VoxelReducerDefinitions[] = {
    { MaxRDI,           "Max density",      "max(rdi)"              },
    { MaxTrust,         "Max trust",        "max(nt - nb)"          },
    { MaxTrustRatio,    "Max trust ratio",  "max((nt - nb)/nt)"     },
    { MaxNi,            "Max hits",         "max(ni)"               },
    { SumRatio,         "Sum ratio",        "sum(ni)/sum(nt - nb)"  },
};

LVOX3_MergeGrids::LVOX3_MergeGrids()
{

}

void LVOX3_MergeGrids::apply(LVOXGridSet *merged,
                             QVector<LVOXGridSet*> *gs,
                             VoxelReducer *reducer)
{
    apply(merged, gs, reducer,
        [](const size_t& i) -> bool {
            Q_UNUSED(i);
            return true;
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

std::unique_ptr<VoxelReducer> LVOX3_MergeGrids::makeReducer(VoxelReducerOptions &opts)
{
    VoxelReducer *reducer = nullptr;

    switch(opts.reducerType) {
    case MaxRDI:
            reducer = new VoxelReducerMaxRDI();
            break;
    case MaxTrust:
            reducer = new VoxelReducerMaxTrust();
            break;
    case MaxTrustRatio:
            reducer = new VoxelReducerMaxTrustRatio();
            break;
    case MaxNi:
            reducer = new VoxelReducerMaxNi();
            break;
    case SumRatio:
            reducer = new VoxelReducerSumRatio();
            break;
    default:
            break;
    };
    if (reducer) {
        reducer->m_opts = opts;
    }
    return std::unique_ptr<VoxelReducer>(reducer);
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
    if (rd > 0) {
        qDebug() << "foo";
    }
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
