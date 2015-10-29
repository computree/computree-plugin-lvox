#include "lvox_stepinterpolatedensitygrid.h"

#include "ct_view/ct_stepconfigurabledialog.h"
#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/ct_outresultmodelgroupcopy.h"
#include "ct_model/inModel/tools/ct_instdmodelpossibility.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"

// Inclusion of standard result class
#include "ct_result/ct_resultgroup.h"

// Inclusion of used ItemDrawable classes
#include "ct_itemdrawable/ct_grid3d.h"

// Alias for indexing in models
#define DEF_resultIn_grids "rgrids"
#define DEF_groupIn_grids "grids"
#define DEF_itemIn_hits "hits"
#define DEF_itemIn_density "density"

#include <math.h>

// Constructor : initialization of parameters
LVOX_StepInterpolateDensityGrids::LVOX_StepInterpolateDensityGrids(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _effectiveRayThresh = 1;
}

// Step description (tooltip of contextual menu)
QString LVOX_StepInterpolateDensityGrids::getStepDescription() const
{
    return tr("Interpolation de voxels de densité");
}

// Step copy method
CT_VirtualAbstractStep* LVOX_StepInterpolateDensityGrids::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new LVOX_StepInterpolateDensityGrids(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void LVOX_StepInterpolateDensityGrids::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy *resultModel = createNewInResultModelForCopy(DEF_resultIn_grids, tr("Grilles"));

    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_groupIn_grids);
    resultModel->addItemModel(DEF_groupIn_grids, DEF_itemIn_hits, CT_Grid3D<int>::staticGetType(), tr("hits"));
    resultModel->addItemModel(DEF_groupIn_grids, DEF_itemIn_density, CT_Grid3D<float>::staticGetType(), tr("density"));
}

// Creation and affiliation of OUT models
void LVOX_StepInterpolateDensityGrids::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *resultModel = createNewOutResultModelToCopy(DEF_resultIn_grids);
    resultModel->addItemModel(DEF_groupIn_grids, _outGridDensity_ModelName, new CT_Grid3D<float>(), tr("density (interpolated)"));
}

// Semi-automatic creation of step parameters DialogBox
void LVOX_StepInterpolateDensityGrids::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();
    configDialog->addInt(tr("Ni minimum pour utiliser la valeur moyenne (0 sinon)"),"",-100000,100000, _effectiveRayThresh);

}

void LVOX_StepInterpolateDensityGrids::compute()
{
    CT_ResultGroup *outResult = getOutResultList().first();

    CT_ResultGroupIterator itGrp(outResult, this, DEF_groupIn_grids);
    while (itGrp.hasNext() && !isStopped())
    {
        CT_AbstractItemGroup *groupIn_grids = (CT_AbstractItemGroup*) itGrp.next();

        const CT_Grid3D<int>* itemIn_hits = (const CT_Grid3D<int>*) groupIn_grids->firstItemByINModelName(this, DEF_itemIn_hits);
        const CT_Grid3D<float>* itemIn_density = (const CT_Grid3D<float>*) groupIn_grids->firstItemByINModelName(this, DEF_itemIn_density);

        if (itemIn_hits != NULL && itemIn_density!=NULL)
        {
            CT_Grid3D<float>* itemOut_density = new CT_Grid3D<float>(_outGridDensity_ModelName.completeName(),
                                                                     outResult,
                                                                     itemIn_density->minX(),
                                                                     itemIn_density->minY(),
                                                                     itemIn_density->minZ(),
                                                                     itemIn_density->xdim(),
                                                                     itemIn_density->ydim(),
                                                                     itemIn_density->zdim(),
                                                                     itemIn_density->resolution(),
                                                                     itemIn_density->NA(),
                                                                     itemIn_density->NA());

            // loop on z levels
            for (size_t zz = 0 ;  zz < itemIn_density->zdim() ; zz++)
            {
                float meanDensity = 0;
                size_t ncells = 0;
                // Compute mean value for the level z
                for (size_t yy = 0 ;  yy < itemIn_density->ydim() ; yy++)
                {
                    for (size_t xx = 0 ;  xx < itemIn_density->xdim() ; xx++)
                    {
                        float value = itemIn_density->value(xx, yy, zz);
                        if (value > 0) // empty cells don't count for the mean density
                        {
                            meanDensity += value;
                            ncells++;
                        }
                    }
                }
                meanDensity /= ncells;

                // set mean value for all NA cells
                for (size_t yy = 0 ;  yy < itemIn_density->ydim() ; yy++)
                {
                    for (size_t xx = 0 ;  xx < itemIn_density->xdim() ; xx++)
                    {
                        float value = itemIn_density->value(xx, yy, zz);
                        if (value < 0) // replace NA values
                        {
                            if (itemIn_hits->value(xx, yy, zz) >= _effectiveRayThresh)
                            {
                                itemOut_density->setValue(xx, yy, zz, meanDensity);
                            } else {
                                itemOut_density->setValue(xx, yy, zz, 0.0);
                            }
                        } else { // else if not a NA : keep the IN value
                            itemOut_density->setValue(xx, yy, zz, value);
                        }
                    }
                }
            }

            itemOut_density->computeMinMax();
            groupIn_grids->addItemDrawable(itemOut_density);
        }
    }



}
