#include "lvox_stepcomputepad.h"

#include "ct_view/ct_stepconfigurabledialog.h"
#include "ct_result/model/inModel/ct_inresultmodelgroup.h"
#include "ct_result/model/outModel/ct_outresultmodelgroup.h"
#include "ct_turn/inTurn/tools/ct_inturnmanager.h"
#include "ct_model/tools/ct_modelsearchhelper.h"
#include "ct_model/inModel/tools/ct_instdmodelpossibility.h"

// Inclusion of standard result class
#include "ct_result/ct_resultgroup.h"

// Inclusion of used ItemDrawable classes
#include "ct_itemdrawable/ct_grid3d.h"
#include "qdebug.h"

// Alias for indexing in models
#define DEF_resultIn_grids "rgrids"
#define DEF_groupIn_grids "grids"
#define DEF_itemIn_density "density"
#define DEF_itemIn_deltaTheoritical "deltaTheoritical"

// Alias for indexing out models
#define DEF_resultOut_grids "rgrids"
#define DEF_groupOut_grids "grids"
#define DEF_itemOut_PAD "PAD"

#include <math.h>

// Constructor : initialization of parameters
LVOX_StepComputePAD::LVOX_StepComputePAD(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _EraseInfinity = false;
    _PADlimit = 100; // 100 m2/voxel
}

// Step description (tooltip of contextual menu)
QString LVOX_StepComputePAD::getStepDescription() const
{
    return tr("6- Calculer grille Plant Area Density");
}

// Step copy method
CT_VirtualAbstractStep* LVOX_StepComputePAD::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new LVOX_StepComputePAD(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void LVOX_StepComputePAD::createInResultModelListProtected()
{
    CT_InResultModelGroup *resultModel = createNewInResultModel(DEF_resultIn_grids, tr("Grilles"));

    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_groupIn_grids);

    resultModel->addItemModel(DEF_groupIn_grids, DEF_itemIn_deltaTheoritical, CT_Grid3D<float>::staticGetType(), tr("delta th."));
    resultModel->addItemModel(DEF_groupIn_grids, DEF_itemIn_density, CT_Grid3D<float>::staticGetType(), tr("density"));
}

// Creation and affiliation of OUT models
void LVOX_StepComputePAD::createOutResultModelListProtected()
{
    CT_OutResultModelGroup *resultModel = createNewOutResultModel(DEF_resultOut_grids, tr("Grilles combinées"));

    resultModel->setRootGroup(DEF_groupOut_grids, new CT_StandardItemGroup(), tr("Grilles"));
    resultModel->addItemModel(DEF_groupOut_grids, DEF_itemOut_PAD, new CT_Grid3D<float>(), tr("PAD"));
}

// Semi-automatic creation of step parameters DialogBox
void LVOX_StepComputePAD::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

     configDialog->addBool(tr("    -> Ecraser les valeurs infinies "),"", "", _EraseInfinity);
     configDialog->addDouble(tr("   -> Effacer les valeurs > à"), "", 0, 100, 2, _PADlimit);
}


void LVOX_StepComputePAD::compute()
{

    float realMax=0;
    CT_ResultGroup* resultIn_grids = getInputResults().first();

    QList<CT_ResultGroup*> outResultList = getOutResultList();
    CT_ResultGroup* resultOut_grids = outResultList.at(0);


    CT_ResultGroupIterator itGrp(resultIn_grids, this, DEF_groupIn_grids);
    while (itGrp.hasNext() && !isStopped())
    {
        CT_AbstractItemGroup *groupIn_grids = (CT_AbstractItemGroup*) itGrp.next();

        const CT_Grid3D<float>* itemIn_density = (const CT_Grid3D<float>*) groupIn_grids->firstItemByINModelName(this, DEF_itemIn_density);
        const CT_Grid3D<float>* itemIn_deltaT = (const CT_Grid3D<float>*) groupIn_grids->firstItemByINModelName(this, DEF_itemIn_deltaTheoritical);

        if (itemIn_density!=NULL && itemIn_deltaT!=NULL)
        {
            float res = itemIn_density->resolution();
            float NAd = itemIn_density->NA();

            CT_StandardItemGroup* groupOut_grids = new CT_StandardItemGroup(DEF_groupOut_grids, resultOut_grids);
            resultOut_grids->addGroup(groupOut_grids);

            size_t xdim = itemIn_density->xdim();
            size_t ydim = itemIn_density->ydim();
            size_t zdim = itemIn_density->zdim();


            CT_Grid3D<float>* itemOut_PAD = new CT_Grid3D<float>(DEF_itemOut_PAD, resultOut_grids,
                                                                 itemIn_density->minX(), itemIn_density->minY(), itemIn_density->minZ(),
                                                                 xdim, ydim, zdim,
                                                                 res, NAd, NAd);
            groupOut_grids->addItemDrawable(itemOut_PAD);

            for (size_t xx = 0 ; xx < xdim ; xx++)
            {
                for (size_t yy = 0 ; yy < ydim ; yy++)
                {
                    for (size_t zz = 0 ; zz < zdim ; zz++)
                    {
                        size_t index;
                        itemIn_density->index(xx, yy, zz, index);
                        float density = itemIn_density->valueAtIndex(index);
                        float deltaT = itemIn_deltaT->valueAtIndex(index);
                        float result = LVOX_StepComputePAD::computePAD(density, res, deltaT, 0.5);

                        // Real max operations
                        if (result==std::numeric_limits<float>::infinity()){
                            if(_EraseInfinity){
                                _InfinityIndexStack.push(index); // Save the voxel index in case of infinity
                            }
                        }
                        else if(realMax<result){realMax=result;} // Or compute the maximum finite result in order to replace the non finite ones

                        // Value assignement
                        if (result>_PADlimit){
                            itemOut_PAD->setValueAtIndex(index,0);
                        }
                        else
                            itemOut_PAD->setValueAtIndex(index,result);
                }
              }
           }

            itemOut_PAD->computeMinMax();
           //PS_LOG->addWarningMessage(LogInterface::reader, "Extremum PAD [" + QString::number(itemOut_PAD->dataMin()) + ";" + QString::number(itemOut_PAD->dataMax()) +"] ");

            if(_EraseInfinity){
                while (!_InfinityIndexStack.isEmpty())
                itemOut_PAD->setValueAtIndex(_InfinityIndexStack.pop(), realMax);
            }

            itemOut_PAD->computeMinMax();
            //PS_LOG->addWarningMessage(LogInterface::reader, "Extremum PAD [" + QString::number(itemOut_PAD->dataMin()) + ";" + QString::number(itemOut_PAD->dataMax()) +"] ");

        }
    }

}

float LVOX_StepComputePAD::computePAD(float density, float res, float D_Nt_mean, float gFunction)
{
    if (density <= 0) {
        //PS_LOG->addWarningMessage(LogInterface::reader, tr("Densité <= 0  [Valeur impossible !!] "));
        return density;
    }
    else if (density >= 1) {
        //PS_LOG->addWarningMessage(LogInterface::reader, tr("Densité = 1 [PAD infini !!] "));
    }

    if (D_Nt_mean == 0) {PS_LOG->addWarningMessage(LogInterface::reader, tr("Distance Nt moyenne = 0 [PAD infini !!] "));}

    return -std::log(1-density)/(D_Nt_mean*0.5);

//    float deltaD_deltaH = 0.07162 * (1 - std::exp(-9.536*(res/D_Nt_mean - 1)));
//    float deltaD = deltaD_deltaH*res;
//    float D_Nt_mean_deltaH = D_Nt_mean / res;

//    return (D_Nt_mean_deltaH - std::sqrt(D_Nt_mean_deltaH*D_Nt_mean_deltaH + 4*deltaD_deltaH*std::log(1-density))) / (2*deltaD*gFunction);
}

