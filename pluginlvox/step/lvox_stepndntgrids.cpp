#include "lvox_stepndntgrids.h"

#include "ct_itemdrawable/ct_grid3d.h"
#include "ct_itemdrawable/tools/iterator/ct_groupiterator.h"
#include "ct_result/ct_resultgroup.h"
#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"

// Alias for indexing models
#define DEF_in_res "result"
#define DEF_in_grp "group"
#define DEF_in_nb "nb (before)"
#define DEF_in_nt "nt (theoretical)"
#define DEF_in_nb_att "isNb"
#define DEF_in_nt_att "isNt"

// Constructor : initialization of parameters
LVOX_StepNdNtGrids::LVOX_StepNdNtGrids(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
}

// Step description (tooltip of contextual menu)
QString LVOX_StepNdNtGrids::getStepDescription() const
{
    return tr("4.5 - Compute nb/nt grid");
}

// Step detailled description
QString LVOX_StepNdNtGrids::getStepDetailledDescription() const
{
    return tr("Compute nb/nt grid");
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
    CT_InResultModelGroupToCopy* in_res = createNewInResultModelForCopy(DEF_in_res, tr("Grilles"));

    in_res->setZeroOrMoreRootGroup();
    in_res->addGroupModel("", DEF_in_grp);

    in_res->addItemModel(DEF_in_grp, DEF_in_nb, CT_Grid3D<int>::staticGetType(), tr("nb (before)"));
    in_res->addItemAttributeModel(DEF_in_nb, DEF_in_nb_att, QList<QString>() << "LVOX_GRD_NB", CT_AbstractCategory::ANY, tr("isNb"), "",
                                  CT_InAbstractModel::C_ChooseOneIfMultiple, CT_InAbstractModel::F_IsOptional);

    in_res->addItemModel(DEF_in_grp, DEF_in_nt, CT_Grid3D<int>::staticGetType(), tr("nt (theoretical)"));
    in_res->addItemAttributeModel(DEF_in_nt, DEF_in_nt_att, QList<QString>() << "LVOX_GRD_NT", CT_AbstractCategory::ANY, tr("isNt"), "",
                                  CT_InAbstractModel::C_ChooseOneIfMultiple, CT_InAbstractModel::F_IsOptional);
}

// Creation and affiliation of OUT models
void LVOX_StepNdNtGrids::createOutResultModelListProtected()
{
    // create a new OUT result that is a copy of the IN result selected by the user
    CT_OutResultModelGroupToCopyPossibilities* out_res = createNewOutResultModelToCopy(DEF_in_res);
    if (out_res != NULL)
    {
        out_res->addItemModel(DEF_in_grp, _nbnt_ModelName, new CT_Grid3D<float>(), tr("nb/nt"));
    }
}

// Semi-automatic creation of step parameters DialogBox
void LVOX_StepNdNtGrids::createPostConfigurationDialog()
{
    // No parameter dialog for this step
}

void LVOX_StepNdNtGrids::compute()
{
    // Get the out res
    CT_ResultGroup* out_res = getOutResultList().first();

    // res browsing
    CT_ResultGroupIterator it(out_res, this, DEF_in_grp);
    while (it.hasNext() && !isStopped())
    {
        CT_StandardItemGroup* out_group = (CT_StandardItemGroup*) it.next();

        const CT_Grid3D<int>* nb = (CT_Grid3D<int>*) out_group->firstItemByINModelName(this, DEF_in_nb);
        const CT_Grid3D<int>* nt = (CT_Grid3D<int>*) out_group->firstItemByINModelName(this, DEF_in_nt);

        CT_Grid3D<float>* nbnt = new CT_Grid3D<float>(_nbnt_ModelName.completeName(), out_res,
            nb->minX(), nb->minY(), nb->minZ(),
            nb->xdim(), nb->ydim(), nb->zdim(),
            nb->resolution(), nb->NA(), nb->NA());

        size_t n = nbnt->nCells();
        for(size_t i = 0; i < n; i++)
        {
            if(nt->valueAtIndex(i)) // avoid division by zero
            {
                nbnt->setValueAtIndex(i, static_cast<float>(nb->valueAtIndex(i))/nt->valueAtIndex(i));
            }
        }
        out_group->addItemDrawable(nbnt);
        nbnt->computeMinMax();
    }
}
