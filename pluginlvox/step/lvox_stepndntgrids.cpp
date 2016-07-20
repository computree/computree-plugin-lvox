#include "lvox_stepndntgrids.h"

#include "ct_itemdrawable/ct_grid3d.h"
#include "ct_itemdrawable/tools/iterator/ct_groupiterator.h"
#include "ct_result/ct_resultgroup.h"
#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"

// Alias for indexing models
#define DEF_in_res "result"
#define DEF_in_grp_ndnt "group"
#define DEF_in_nd "nb (before)"
#define DEF_in_nt "nt (theoretical)"


// Constructor : initialization of parameters
LVOX_StepNdNtGrids::LVOX_StepNdNtGrids(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
}

// Step description (tooltip of contextual menu)
QString LVOX_StepNdNtGrids::getStepDescription() const
{
    return tr("4.5 - Compute nd/nt grid");
}

// Step detailled description
QString LVOX_StepNdNtGrids::getStepDetailledDescription() const
{
    return tr("Compute nd/nt grid");
}

// Step URL
QString LVOX_StepNdNtGrids::getStepURL() const
{
    //return tr("STEP URL HERE");
    return CT_AbstractStep::getStepURL(); //by default URL of the plugin
}

// Step copy method
CT_VirtualAbstractStep* LVOX_StepNdNtGrids::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new LVOX_StepNdNtGrids(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void LVOX_StepNdNtGrids::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy *resultModel = createNewInResultModelForCopy(DEF_in_res, tr("Grilles"));

    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_in_grp_ndnt);

    resultModel->addItemModel(DEF_in_grp_ndnt, DEF_in_nd, CT_Grid3D<int>::staticGetType(), tr("nb (before)"));
//    resultModel->addItemAttributeModel(DEFin_nd, DEFin_nd, QList<QString>() << "LVOX_GRD_ND", CT_AbstractCategory::ANY, tr("before"),
//        "", CT_InAbstractModel::C_ChooseOneIfMultiple, CT_InAbstractModel::F_IsOptional);

    resultModel->addItemModel(DEF_in_grp_ndnt, DEF_in_nt, CT_Grid3D<int>::staticGetType(), tr("nt (theoretical)"));
//    resultModel->addItemAttributeModel(DEFin_nt, DEFin_nt, QList<QString>() << "LVOX_GRD_NT", CT_AbstractCategory::ANY, tr("theoretical"),
//        "", CT_InAbstractModel::C_ChooseOneIfMultiple, CT_InAbstractModel::F_IsOptional);
}

// Creation and affiliation of OUT models
void LVOX_StepNdNtGrids::createOutResultModelListProtected()
{
    // create a new OUT result that is a copy of the IN result selected by the user
    CT_OutResultModelGroupToCopyPossibilities *res = createNewOutResultModelToCopy(DEF_in_res);
    if (res != NULL)
    {
        res->addItemModel(DEF_in_grp_ndnt, _ndnt_ModelName, new CT_Grid3D<float>(), tr("ndovernt"));
//        res->addItemAttributeModel(_ndnt_ModelName, _DensityFlag_ModelName,
//                                   new CT_StdItemAttributeT<bool>("LVOX_GRD_DENSITY"), tr("isDensity"));
    }
}

// Semi-automatic creation of step parameters DialogBox
void LVOX_StepNdNtGrids::createPostConfigurationDialog()
{
    // No parameter dialog for this step
}

void LVOX_StepNdNtGrids::compute()
{
    // Get the input
//    CT_ResultGroup* in = getInputResults().first();

    // Get the out res
    CT_ResultGroup* out = getOutResultList().first();

    // res browsing
    CT_ResultGroupIterator it(out, this, DEF_in_grp_ndnt);
    while (it.hasNext() && !isStopped())
    {
        CT_StandardItemGroup* group = (CT_StandardItemGroup*) it.next();

        const CT_Grid3D<int>* nd = (CT_Grid3D<int>*) group->firstItemByINModelName(this, DEF_in_nd);
        const CT_Grid3D<int>* nt = (CT_Grid3D<int>*) group->firstItemByINModelName(this, DEF_in_nt);

        CT_Grid3D<float>* ndnt = new CT_Grid3D<float>(_ndnt_ModelName.completeName(), out,
            nd->minX(), nd->minY(), nd->minZ(),
            nd->xdim(), nd->ydim(), nd->zdim(),
            nd->resolution(), nd->NA(), nd->NA());

        size_t n = ndnt->nCells();
        for(size_t i = 0; i < n; i++)
        {
            if(nt->valueAtIndex(i))
            {
                int a = nd->valueAtIndex(i);
                int b = nt->valueAtIndex(i);
                float val = static_cast<float>(a)/b;
                ndnt->setValueAtIndex(i, val);
            }
        }
        group->addItemDrawable(ndnt);
        ndnt->computeMinMax();
    }
}
