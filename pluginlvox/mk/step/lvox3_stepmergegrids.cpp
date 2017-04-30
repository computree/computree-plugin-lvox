#include <QDialog>

#include "lvox3_stepmergegrids.h"
#include "ct_view/tools/ct_configurablewidgettodialog.h"
#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"
#include "ct_result/ct_resultgroup.h"
#include "mk/tools/lvox3_gridtype.h"
#include "mk/tools/lvox3_gridtools.h"
#include "mk/tools/lvox3_utils.h"
#include "mk/tools/lvox3_mergegrids.h"
#include "mk/tools/worker/lvox3_mergegridsworker.h"
#include "mk/view/mergegridsconfiguration.h"

static const QString settingsRootName("LVOX3_StepMergeGrids");
static const QString settingsEffectiveRayThreshold("effectiveRayThreshold");
static const QString settingsIgnoreVoxelZeroDensity("ignoreVoxelZeroDensity");
static const QString settingsReducerLabel("reducerLabel");

LVOX3_StepMergeGrids::LVOX3_StepMergeGrids(CT_StepInitializeData &dataInit) :
    CT_AbstractStepCanBeAddedFirst(dataInit)
{

}

QString LVOX3_StepMergeGrids::getStepDescription() const
{
    return tr("6 - merge grids from multiple scans (LVOX 3)");
}

CT_VirtualAbstractStep *LVOX3_StepMergeGrids::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new LVOX3_StepMergeGrids(dataInit);
}

/*
 * FIXME: should use a label instead of an enum int for the reducer type. If
 * the list of reducers is modified, then the setting will be invalid, but the
 * user will not get the configuration dialog.
 */

SettingsNodeGroup* LVOX3_StepMergeGrids::getAllSettings() const
{
    SettingsNodeGroup *root = CT_AbstractStepCanBeAddedFirst::getAllSettings();
    SettingsNodeGroup *group = new SettingsNodeGroup(settingsRootName);
    group->addValue(new SettingsNodeValue(settingsEffectiveRayThreshold, m_reducerOptions.effectiveRaysThreshold));
    group->addValue(new SettingsNodeValue(settingsIgnoreVoxelZeroDensity, m_reducerOptions.ignoreVoxelZeroDensity));
    group->addValue(new SettingsNodeValue(settingsReducerLabel, m_reducerOptions.reducerLabel));
    root->addGroup(group);
    return root;
}

bool LVOX3_StepMergeGrids::setAllSettings(const SettingsNodeGroup* settings)
{
    if(!CT_AbstractStepCanBeAddedFirst::setAllSettings(settings))
        return false;

    QList<SettingsNodeGroup*> groupList = settings->groupsByTagName(settingsRootName);

    if(groupList.isEmpty())
        return false;

    SettingsNodeValue* s1, *s2, *s3;
    SettingsNodeGroup* group = groupList.first();
    s1 = group->firstValueByTagName(settingsEffectiveRayThreshold);
    s2 = group->firstValueByTagName(settingsIgnoreVoxelZeroDensity);
    s3 = group->firstValueByTagName(settingsReducerLabel);

    if (!(s1 && s2 && s3)) {
        return false;
    }

    m_reducerOptions.effectiveRaysThreshold = s1->value().toInt();
    m_reducerOptions.ignoreVoxelZeroDensity = s2->value().toBool();
    m_reducerOptions.reducerLabel = s3->value().toString();

    LVOX3_MergeGrids mg;
    std::unique_ptr<VoxelReducer> vr = mg.makeReducer(m_reducerOptions.reducerLabel);
    if (!vr) {
        return false;
    }
    return true;
}

void LVOX3_StepMergeGrids::createInResultModelListProtected()
{
    /*
     * Requirements:
     *  - relative density 3D grid
     *  - theoretical rays 3D grid
     *  - blocked rays 3D grid
     *  - incident rays 3D grid
     */

    CT_InResultModelGroupToCopy *resultModel = createNewInResultModelForCopy(
            DEF_SearchInResult, tr("Grids"), "", true);
    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_SearchInGroup,
            CT_AbstractItemGroup::staticGetType(), tr("Group"));
    LVOX3_Utils::requireGrid(resultModel, Theoretic);
    LVOX3_Utils::requireGrid(resultModel, Hits);
    LVOX3_Utils::requireGrid(resultModel, Before);
    //LVOX3_Utils::requireGrid(resultModel, Density);

    // FIXME: attribute required on the grid for matching
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInGrid,
            lvox::Grid3Df::staticGetType(), tr("Density grid"));
}

bool LVOX3_StepMergeGrids::postConfigure()
{
    MergeGridsConfiguration widget;
    widget.setOptions(m_reducerOptions);

    if(CT_ConfigurableWidgetToDialog::exec(&widget) == QDialog::Rejected)
        return false;

    m_reducerOptions = widget.getOptions();

    qDebug() << "reducer options:"
             << m_reducerOptions.reducerLabel
             << m_reducerOptions.effectiveRaysThreshold
             << m_reducerOptions.ignoreVoxelZeroDensity;

    setSettingsModified(true);
    return true;
}

void LVOX3_StepMergeGrids::createPostConfigurationDialog()
{
}

void LVOX3_StepMergeGrids::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *res = createNewOutResultModelToCopy(DEF_SearchInResult);

    if (!res)
        return;

    res->addItemModel(DEF_SearchInGroup,
                      m_mergedGridHits,
                      new lvox::Grid3Di(),
                      tr("Hits (merged)"));
    res->addItemAttributeModel(m_mergedGridHits,
                               m_mergedNiFlag,
                               new CT_StdItemAttributeT<bool>(DEF_LVOX_GRD_NI),
                               tr("isNi"));

    res->addItemModel(DEF_SearchInGroup,
                      m_mergedGridTheoretic,
                      new lvox::Grid3Di(),
                      tr("Theoretic (merged)"));
    res->addItemAttributeModel(m_mergedGridTheoretic,
                               m_mergedNtFlag,
                               new CT_StdItemAttributeT<bool>(DEF_LVOX_GRD_NT),
                               tr("isNt"));

    res->addItemModel(DEF_SearchInGroup,
                      m_mergedGridBefore,
                      new lvox::Grid3Di(),
                      tr("Before (merged)"));
    res->addItemAttributeModel(m_mergedGridBefore,
                               m_mergedNbFlag,
                               new CT_StdItemAttributeT<bool>(DEF_LVOX_GRD_NB),
                               tr("isNb"));

    res->addItemModel(DEF_SearchInGroup,
                      m_mergedGridDensity,
                      new lvox::Grid3Df(),
                      tr("Density (merged)"));

}

void LVOX3_StepMergeGrids::compute()
{
    LVOX3_MergeGrids mg;
    std::unique_ptr<VoxelReducer> reducer = mg.makeReducer(m_reducerOptions.reducerLabel);
    if (!reducer) {
        PS_LOG->addFatalMessage(this, "Cannot find the requested grid reducer " + m_reducerOptions.reducerLabel);
        return;
    }

    CT_ResultGroup* outResult = getOutResultList().first();
    CT_ResultGroupIterator itGrp(outResult, this, DEF_SearchInGroup);

    QVector<LVOXGridSet*> gs;
    CT_StandardItemGroup *groupOut = NULL;
    while(itGrp.hasNext() && !isStopped()) {
        CT_StandardItemGroup *group = dynamic_cast<CT_StandardItemGroup*>((CT_AbstractItemGroup*)itGrp.next());
        if (!groupOut) {
            groupOut = group;
        }

        lvox::Grid3Df *grid_density = dynamic_cast<lvox::Grid3Df*>(group->firstItemByINModelName(this, DEF_SearchInGrid));
        lvox::Grid3Di *grid_nt = dynamic_cast<lvox::Grid3Di*>(group->firstItemByINModelName(this, DEF_Nt));
        lvox::Grid3Di *grid_nb = dynamic_cast<lvox::Grid3Di*>(group->firstItemByINModelName(this, DEF_Nb));
        lvox::Grid3Di *grid_ni = dynamic_cast<lvox::Grid3Di*>(group->firstItemByINModelName(this, DEF_Ni));

        if(!(grid_density && grid_nt && grid_nb && grid_ni))
            continue;

        qDebug() << "density grid info: " << grid_density->getInfo();
        qDebug() << "theoric grid info: " << grid_nt->getInfo();
        qDebug() << "blocked grid info: " << grid_nb->getInfo();
        qDebug() << "hits    grid info: " << grid_ni->getInfo();

        gs.append(new LVOXGridSet{grid_ni, grid_nt, grid_nb, grid_density});

    }

    if (gs.isEmpty())
        return;

    LVOXGridSet *a = gs.first();
    LVOXGridSet merged;

    merged.ni = (lvox::Grid3Di*) a->ni->copy(m_mergedGridHits.completeName(),
                outResult, CT_ResultCopyModeList());
    merged.nt = (lvox::Grid3Di*) a->nt->copy(m_mergedGridTheoretic.completeName(),
                outResult, CT_ResultCopyModeList());
    merged.nb = (lvox::Grid3Di*) a->nb->copy(m_mergedGridBefore.completeName(),
                outResult, CT_ResultCopyModeList());
    merged.rd = (lvox::Grid3Df*) a->rd->copy(m_mergedGridDensity.completeName(),
                outResult, CT_ResultCopyModeList());

    LVOX3_MergeGridsWorker worker(&merged, &gs, reducer.get());

    connect(&worker, SIGNAL(progressChanged(int)), this, SLOT(workerProgressChanged(int)), Qt::DirectConnection);
    connect(this, SIGNAL(stopped()), &worker, SLOT(cancel()), Qt::DirectConnection);
    worker.compute();

    merged.ni->computeMinMax();
    merged.nt->computeMinMax();
    merged.nb->computeMinMax();
    merged.rd->computeMinMax();

    groupOut->addItemDrawable(merged.ni);
    groupOut->addItemDrawable(merged.nt);
    groupOut->addItemDrawable(merged.nb);
    groupOut->addItemDrawable(merged.rd);

    qDeleteAll(gs);
}

void LVOX3_StepMergeGrids::workerProgressChanged(int p)
{
    setProgress(p);
}

