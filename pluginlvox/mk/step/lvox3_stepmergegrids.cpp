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
#include "mk/view/mergegridsconfiguration.h"

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
             << m_reducerOptions.reducerType
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

}

void LVOX3_StepMergeGrids::compute()
{

}

void LVOX3_StepMergeGrids::workerProgressChanged(int p)
{

}

