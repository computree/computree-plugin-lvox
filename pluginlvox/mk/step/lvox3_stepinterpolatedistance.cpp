#include <memory>

#include "lvox3_stepinterpolatedistance.h"

#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"
#include "ct_result/ct_resultgroup.h"
#include "ct_itemdrawable/ct_scene.h"
#include "ct_view/ct_stepconfigurabledialog.h"
#include "mk/tools/lvox3_gridtype.h"
#include "mk/tools/lvox3_gridtools.h"
#include "mk/tools/worker/lvox3_interpolatedistance.h"
#include "mk/tools/worker/lvox3_interpolatetrustfactor.h"

#define DEF_SearchInResult      "r"
#define DEF_SearchInGrid        "grid"
#define DEF_SearchInGroup       "gr"
#define DEF_Nt "theoretical"
#define DEF_Nb "before"
#define DEF_Ni "hits"
#define DEF_inATTisNi "isNi"
#define DEF_inATTisNt "isNt"
#define DEF_inATTisNb "isNb"

using namespace std;

LVOX3_StepInterpolateDistance::LVOX3_StepInterpolateDistance(CT_StepInitializeData &dataInit)
    : CT_AbstractStep(dataInit),
      m_interpolateRadius(1.0),
      m_enableInterpolateDistance(true),
      m_interpolateDensityThreshold(0.0),
      m_interpolatePower(0),
      m_enableInterpolateTrust(true),
      m_trustLowThreshold(10),
      m_trustHighThreshold(30)
{
}

QString LVOX3_StepInterpolateDistance::getStepDescription() const
{
    return tr("4 - NODATA interpolation (LVOX 3)");
}

CT_VirtualAbstractStep*
LVOX3_StepInterpolateDistance::createNewInstance(CT_StepInitializeData &dataInit)
{
    // Creates an instance of this step
    return new LVOX3_StepInterpolateDistance(dataInit);
}

void LVOX3_StepInterpolateDistance::createInResultModelListProtected()
{
    /*
     * Requirements:
     *  - relative density 3D grid
     *  - theoretical rays 3D grid
     *  - blocked rays 3D grid
     */

    CT_InResultModelGroupToCopy *resultModel = createNewInResultModelForCopy(
            DEF_SearchInResult, tr("Grids"), "", true);
    resultModel->setZeroOrMoreRootGroup();

    /*
     * The main density grid
     */
    resultModel->addGroupModel("", DEF_SearchInGroup,
            CT_AbstractItemGroup::staticGetType(), tr("Group"));
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInGrid,
            lvox::Grid3Df::staticGetType(), tr("Density grid"));

    /*
     * The theoretical and before grid required for the trust interpolation.
     *
     * FIXME: the user must select the proper grids, otherwise the computation
     * will not be valid. Is there a way to specifically request a default
     * grid?
     *
     * ATM, the output grids are matched in order, and the default order (Nt,
     * Nb) is correct, but this is obviously flaky, the user may make a mistake easily.
     *
     * In lvox_stepcombinedentisygrids.cpp, an item attribute is requested with
     * addItemAttributeModel, but lvox3_stepgenericcomputegrids.cpp does not
     * add such attributes that would allow matching.
     */

    resultModel->addItemModel(DEF_SearchInGroup, DEF_Nt,
            lvox::Grid3Di::staticGetType(), tr("theoretical"), "",
            CT_InAbstractModel::C_ChooseOneIfMultiple,
            CT_InAbstractModel::F_IsOptional);

    resultModel->addItemModel(DEF_SearchInGroup, DEF_Nb,
            lvox::Grid3Di::staticGetType(), tr("before"), "",
            CT_InAbstractModel::C_ChooseOneIfMultiple,
            CT_InAbstractModel::F_IsOptional);
}

void LVOX3_StepInterpolateDistance::createPostConfigurationDialog()
{
    /*
     * FIXME:
     * - The settings for an algorithm are not disabled if the corresponding
     *   algorithm is not checked.
     * - The low/high range bound is not validated, meaning that the user can
     *   enter a lower bound greater than the higher bound.
     * - We should use a combobox instead of the checkbox for each algorithm.
     *   However, I haven't found a way to register a signal to it (not returned from the add)
     * - addWidget() is more generic, but is protected.
     */
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addText(tr("Common parameters"));
    configDialog->addDouble(tr("Minimum density"), tr(""), 0.0, 1.0, 3,
            m_interpolateDensityThreshold, 0.0, tr("Minimum neighbor value to consider the voxel in the average"));
    configDialog->addDouble(tr("Interpolation radius"), tr("meter"), 0.0, 10000.0, 3, m_interpolateRadius);

    configDialog->addEmpty();

    configDialog->addBool(tr("Enable distance interpolation"), "", "", m_enableInterpolateDistance);
    configDialog->addInt(tr("Decay power"), tr(""), 0, 100, m_interpolatePower);

    configDialog->addEmpty();

    configDialog->addBool(tr("Enable trust interpolation"), "", "", m_enableInterpolateTrust);
    configDialog->addInt(tr("Lower bound"), tr("effective rays (Ni - Nb)"), 0, 1000, m_trustLowThreshold, tr("Voxel with lower effective rays are not trusted"));
    configDialog->addInt(tr("Higher bound"), tr("effective rays (Ni - Nb)"), 0, 1000, m_trustHighThreshold, tr("Voxel with higher effective rays are fully trusted"));

}

void LVOX3_StepInterpolateDistance::createOutResultModelListProtected()
{
    // create a new OUT result that is a copy of the IN result selected by the user
    CT_OutResultModelGroupToCopyPossibilities *resultModel =
            createNewOutResultModelToCopy(DEF_SearchInResult);

    /*
     * addItemModel() declares the outputs, the object lvox::Grid3Df is only a
     * stub and is not actually used.
     *
     * FIXME: depending on the interpolation algorithm selected by the user,
     * only one grid will be produced and the output name should be different.
     */
    if(resultModel != NULL) {

        resultModel->addItemModel(DEF_SearchInGroup,
                m_outInterpolatedDistanceGridModelName,
                new lvox::Grid3Df(),
                tr("Density - distance interpolation"));

        resultModel->addItemModel(DEF_SearchInGroup,
                m_outInterpolatedTrustGridModelName,
                new lvox::Grid3Df(),
                tr("Density - trust interpolation"));
    }
}

/*
 * WorkItem is a plain object used locally to hold multiple values for the compute method.
 * Owns the worker pointer.
 */
struct WorkItem {
    WorkItem(const QString &name, LVOX3_Worker *worker, lvox::Grid3Df *result) :
        m_name(name), m_worker(worker), m_result(result) { }
    QString m_name;
    unique_ptr<LVOX3_Worker> m_worker;
    lvox::Grid3Df *m_result;
};

void LVOX3_StepInterpolateDistance::compute()
{
    CT_ResultGroup* outResult = getOutResultList().first();

    CT_ResultGroupIterator itGrp(outResult, this, DEF_SearchInGroup);

    while(itGrp.hasNext() && !isStopped()) {
        /*
         * Two casts are required, the group iterator returns const objects and
         * because addItemDrawable() is called, it has to be non const.
         */
        CT_StandardItemGroup *group = dynamic_cast<CT_StandardItemGroup*>((CT_AbstractItemGroup*)itGrp.next());

        lvox::Grid3Df *grid_density = dynamic_cast<lvox::Grid3Df*>(group->firstItemByINModelName(this, DEF_SearchInGrid));
        lvox::Grid3Di *grid_nt = dynamic_cast<lvox::Grid3Di*>(group->firstItemByINModelName(this, DEF_Nt));
        lvox::Grid3Di *grid_nb = dynamic_cast<lvox::Grid3Di*>(group->firstItemByINModelName(this, DEF_Nb));

        qDebug() << "density grid info: " << grid_density->getInfo();
        qDebug() << "theoric grid info: " << grid_nt->getInfo();
        qDebug() << "blocked grid info: " << grid_nb->getInfo();

        if(!(grid_density && grid_nt && grid_nb))
            continue;

        vector<unique_ptr<WorkItem>> work;

        if (m_enableInterpolateDistance) {
            lvox::Grid3Df *out = (lvox::Grid3Df*) grid_density->copy(
                    m_outInterpolatedDistanceGridModelName.completeName(),
                    outResult, CT_ResultCopyModeList());
            LVOX3_Worker *worker = new LVOX3_InterpolateDistance(
                    grid_density, out, m_interpolateRadius,
                    m_interpolatePower, m_interpolateDensityThreshold);
            work.push_back(unique_ptr<WorkItem>(new WorkItem("Density interpolated (distance)", worker, out)));
        }

        if (m_enableInterpolateTrust) {
            lvox::Grid3Df *out = (lvox::Grid3Df*) grid_density->copy(
                    m_outInterpolatedTrustGridModelName.completeName(),
                    outResult, CT_ResultCopyModeList());
            LVOX3_Worker *worker = new LVOX3_InterpolateTrustFactor(
                    grid_density, grid_nt, grid_nb, out, m_interpolateRadius,
                    m_trustLowThreshold, m_trustHighThreshold);
            work.push_back(unique_ptr<WorkItem>(new WorkItem("Density interpolated (trust)", worker, out)));
        }

        for (unique_ptr<WorkItem> &item: work) {
            item->m_worker->compute();
            item->m_result->computeMinMax();

            /*
             * The object added knows its modelName and associated result group.
             */
            group->addItemDrawable(item->m_result);

            /*
             * We change the model name to match the currently selected
             * interpolation method.
             *
             * FIXME: that actually doesn't work.
             */
            //item->m_result->setDisplayableName(item->m_name);

        }
    }
}

void LVOX3_StepInterpolateDistance::workerProgressChanged(int p)
{
    setProgress(p);
}
