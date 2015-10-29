#include "lvox_stepcomputeprofile.h"

#include "ct_view/ct_stepconfigurabledialog.h"
#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/ct_outresultmodelgroupcopy.h"
#include "ct_model/inModel/tools/ct_instdmodelpossibility.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"

// Inclusion of standard result class
#include "ct_result/ct_resultgroup.h"

// Inclusion of used ItemDrawable classes
#include "ct_itemdrawable/ct_grid3d.h"
#include "ct_itemdrawable/ct_profile.h"

// Alias for indexing in models
#define DEF_resultIn_grids "rgrids"
#define DEF_groupIn_grids "grids"
#define DEF_itemIn_grid "density"

#include <math.h>

// Constructor : initialization of parameters
LVOX_StepComputeProfile::LVOX_StepComputeProfile(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _min = 0;
}

// Step description (tooltip of contextual menu)
QString LVOX_StepComputeProfile::getStepDescription() const
{
    return tr("Profil vertical de voxels");
}

// Step copy method
CT_VirtualAbstractStep* LVOX_StepComputeProfile::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new LVOX_StepComputeProfile(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void LVOX_StepComputeProfile::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy *resultModel = createNewInResultModelForCopy(DEF_resultIn_grids, tr("Grilles"));

    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_groupIn_grids);
    resultModel->addItemModel(DEF_groupIn_grids, DEF_itemIn_grid, CT_AbstractGrid3D::staticGetType(), tr("Grille 3D"));
}

// Creation and affiliation of OUT models
void LVOX_StepComputeProfile::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *resultModel = createNewOutResultModelToCopy(DEF_resultIn_grids);
    resultModel->addItemModel(DEF_groupIn_grids, _outProfile_ModelName, new CT_Profile<double>(), tr("Profil"));
}

// Semi-automatic creation of step parameters DialogBox
void LVOX_StepComputeProfile::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addDouble(tr("Ne prendre en compte que les valeurs > à"), "", -99999, 99999, 2, _min);
}

void LVOX_StepComputeProfile::compute()
{
    CT_ResultGroup *outResult = getOutResultList().first();

    CT_ResultGroupIterator itGrp(outResult, this, DEF_groupIn_grids);
    while (itGrp.hasNext() && !isStopped())
    {
        CT_AbstractItemGroup *groupIn_grids = (CT_AbstractItemGroup*) itGrp.next();

        const CT_AbstractGrid3D* inGrid = (const CT_AbstractGrid3D*) groupIn_grids->firstItemByINModelName(this, DEF_itemIn_grid);

        if (inGrid!=NULL)
        {
            bool ok;
            double NAval = inGrid->NAAsString().toDouble(&ok);
            if (!ok) {NAval = -std::numeric_limits<double>::max();}

            CT_Profile<double>* outProfile = new CT_Profile<double>(_outProfile_ModelName.completeName(),
                                                                     outResult,
                                                                     inGrid->minX(),
                                                                     inGrid->minY(),
                                                                     inGrid->minZ(),
                                                                     0,
                                                                     0,
                                                                     1,
                                                                     inGrid->zdim(),
                                                                     inGrid->resolution(),
                                                                     NAval,
                                                                     0.0);

            // loop on z levels
            for (size_t zz = 0 ;  zz < inGrid->zdim() ; zz++)
            {
                for (size_t yy = 0 ;  yy < inGrid->ydim() ; yy++)
                {
                    for (size_t xx = 0 ;  xx < inGrid->xdim() ; xx++)
                    {
                        size_t index;
                        inGrid->index(xx, yy, zz, index);
                        double value = inGrid->valueAtIndexAsDouble(index);

                        if (value > _min)
                        {
                            outProfile->addValueAtIndex(zz, value);
                        }
                    }
                }
            }

            outProfile->computeMinMax();
            groupIn_grids->addItemDrawable(outProfile);
        }
    }



}
