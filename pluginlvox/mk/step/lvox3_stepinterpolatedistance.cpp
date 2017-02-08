#include "lvox3_stepinterpolatedistance.h"

#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"
#include "ct_result/ct_resultgroup.h"
#include "ct_itemdrawable/ct_scene.h"
#include "ct_view/ct_stepconfigurabledialog.h"
#include "mk/tools/lvox3_gridtype.h"
#include "mk/tools/lvox3_gridtools.h"
#include "mk/tools/worker/lvox3_interpolatedistance.h"

#define DEF_SearchInResult      "r"
#define DEF_SearchInGrid        "grid"
#define DEF_SearchInGroup       "gr"

LVOX3_StepInterpolateDistance::LVOX3_StepInterpolateDistance(CT_StepInitializeData &dataInit)
    : CT_AbstractStep(dataInit), m_interpolateRadius(1.0), m_interpolatePower(0)
{
}

QString LVOX3_StepInterpolateDistance::getStepDescription() const
{
    return tr("4 - No-data interpolation w/ radius (LVOX 3)");
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
     */

    CT_InResultModelGroupToCopy *resultScan = createNewInResultModelForCopy(DEF_SearchInResult, tr("Grids"), "", true);
    resultScan->setZeroOrMoreRootGroup();
    resultScan->addGroupModel("", DEF_SearchInGroup, CT_AbstractItemGroup::staticGetType(), tr("Group"));
    resultScan->addItemModel(DEF_SearchInGroup, DEF_SearchInGrid, lvox::Grid3Df::staticGetType(), tr("Density grid"));
}

void LVOX3_StepInterpolateDistance::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addDouble(tr("Interpolation radius"), tr("meter"), 0.0, 10000.0, 2, m_interpolateRadius);
    configDialog->addInt(tr("Decay power"), tr(""), 0, 100, m_interpolatePower);
}

void LVOX3_StepInterpolateDistance::createOutResultModelListProtected()
{
    // create a new OUT result that is a copy of the IN result selected by the user
    CT_OutResultModelGroupToCopyPossibilities *resultModel = createNewOutResultModelToCopy(DEF_SearchInResult);

    /*
     * addItemModel() declares the outputs, the object lvox::Grid3Df is only a
     * stub and is not actually used.
     */
    if(resultModel != NULL) {
        resultModel->addItemModel(DEF_SearchInGroup, m_outInterpolatedGridModelName, new lvox::Grid3Df(), tr("Density interpolated"));
    }
}

void LVOX3_StepInterpolateDistance::compute()
{
    CT_ResultGroup* outResult = getOutResultList().first();

    CT_ResultGroupIterator itGrp(outResult, this, DEF_SearchInGroup);

    while(itGrp.hasNext() && !isStopped()) {
        CT_StandardItemGroup *group = dynamic_cast<CT_StandardItemGroup*>((CT_AbstractItemGroup*)itGrp.next());

        lvox::Grid3Df* igrid = dynamic_cast<lvox::Grid3Df*>(group->firstItemByINModelName(this, DEF_SearchInGrid));

        if(igrid) {
            /**/

            lvox::Grid3Df *outputGrid = new lvox::Grid3Df(m_outInterpolatedGridModelName.completeName(),
                                                    outResult, igrid->minX(), igrid->minY(), igrid->minZ(),
                                                    igrid->xdim(), igrid->ydim(), igrid->zdim(),
                                                    igrid->resolution(), 0, 0);


            LVOX3_InterpolateDistance worker(igrid, outputGrid, m_interpolateRadius, m_interpolatePower);

            worker.compute();

            /*
             * The object added knows its modelName and associated result group.
             */
            group->addItemDrawable(outputGrid);
        }
    }
}

void LVOX3_StepInterpolateDistance::workerProgressChanged(int p)
{
    setProgress(p);
}
