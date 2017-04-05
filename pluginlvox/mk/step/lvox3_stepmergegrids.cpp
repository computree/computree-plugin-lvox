#include "lvox3_stepmergegrids.h"

#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"
#include "ct_result/ct_resultgroup.h"
#include "mk/tools/lvox3_gridtype.h"
#include "mk/tools/lvox3_gridtools.h"
#include "mk/tools/lvox3_utils.h"

LVOX3_StepMergeGrids::LVOX3_StepMergeGrids(CT_StepInitializeData &dataInit) :
    CT_AbstractStep(dataInit)
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
    LVOX3_Utils::requireGrid(resultModel, Blocked);
    //LVOX3_Utils::requireGrid(resultModel, Density);


    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInGrid,
            lvox::Grid3Df::staticGetType(), tr("Density grid"));
    /*
    resultModel->addItemModel(DEF_SearchInGroup, DEF_Nt,
            lvox::Grid3Di::staticGetType(), tr("theoretical"));
    resultModel->addItemAttributeModel(DEF_Nt, "ntFlagModelName",
                                       QList<QString>() << "LVOX_GRD_NT",
                                       CT_AbstractCategory::ANY, tr("isNt"));

    resultModel->addItemModel(DEF_SearchInGroup, DEF_Nb,
            lvox::Grid3Di::staticGetType(), tr("before"));
    resultModel->addItemAttributeModel(DEF_Nb, "nbFlagModelName",
                                       QList<QString>() << "LVOX_GRD_NB",
                                       CT_AbstractCategory::ANY, tr("isNb"));

    resultModel->addItemModel(DEF_SearchInGroup, DEF_Ni,
            lvox::Grid3Di::staticGetType(), tr("hits"));
    resultModel->addItemAttributeModel(DEF_Ni, "niFlagModelName",
                                       QList<QString>() << "LVOX_GRD_NI",
                                       CT_AbstractCategory::ANY, tr("isNi"));
    */
}

void LVOX3_StepMergeGrids::createPostConfigurationDialog()
{

}

void LVOX3_StepMergeGrids::createOutResultModelListProtected()
{

}

void LVOX3_StepMergeGrids::compute()
{

}

void LVOX3_StepMergeGrids::workerProgressChanged(int p)
{

}

