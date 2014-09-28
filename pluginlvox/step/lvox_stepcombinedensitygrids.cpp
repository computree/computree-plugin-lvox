#include "lvox_stepcombinedensitygrids.h"

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
#define DEF_itemIn_hits "hits"
#define DEF_itemIn_theorical "theorical"
#define DEF_itemIn_before "before"
#define DEF_itemIn_density "density"

// Alias for indexing out models
#define DEF_resultOut_grids "rgrids"
#define DEF_groupOut_grids "grids"
#define DEF_itemOut_hits "hits"
#define DEF_itemOut_theorical "theorical"
#define DEF_itemOut_before "before"
#define DEF_itemOut_density "density"
#define DEF_itemOut_scanId "scanId"

// Constructor : initialization of parameters
LVOX_StepCombineDensityGrids::LVOX_StepCombineDensityGrids(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _mode = 0;
    _effectiveRayThresh = 10;
}

// Step description (tooltip of contextual menu)
QString LVOX_StepCombineDensityGrids::getStepDescription() const
{
    return "Combines density grids from different points of view";
}

// Step copy method
CT_VirtualAbstractStep* LVOX_StepCombineDensityGrids::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new LVOX_StepCombineDensityGrids(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void LVOX_StepCombineDensityGrids::createInResultModelListProtected()
{
    CT_InResultModelGroup *resultModel = createNewInResultModel(DEF_resultIn_grids, tr("Grilles"));

    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_groupIn_grids);

    resultModel->addItemModel(DEF_groupIn_grids, DEF_itemIn_hits, CT_Grid3D<int>::staticGetType(), tr("hits"), "",
                              CT_InAbstractModel::C_ChooseOneIfMultiple, CT_InAbstractModel::F_IsOptional);

    resultModel->addItemModel(DEF_groupIn_grids, DEF_itemIn_theorical, CT_Grid3D<int>::staticGetType(), tr("theorical"), "",
                              CT_InAbstractModel::C_ChooseOneIfMultiple, CT_InAbstractModel::F_IsOptional);

    resultModel->addItemModel(DEF_groupIn_grids, DEF_itemIn_before, CT_Grid3D<int>::staticGetType(), tr("before"), "",
                              CT_InAbstractModel::C_ChooseOneIfMultiple, CT_InAbstractModel::F_IsOptional);

    resultModel->addItemModel(DEF_groupIn_grids, DEF_itemIn_density, CT_Grid3D<double>::staticGetType(), tr("density"));
}

// Creation and affiliation of OUT models
void LVOX_StepCombineDensityGrids::createOutResultModelListProtected()
{
    QString modeString;
    if (_mode == maxDensity)    {modeString = "max(density)";}
    else if (_mode == maxNt_Nb) {modeString = "max(nt-nb)";}
    else if (_mode == maxNi)    {modeString = "max(ni)";}
    else if (_mode == sumNiSumNtNb)    {modeString = "sum(ni)/sum(nt-nb)";}

    CT_OutResultModelGroup *resultModel = createNewOutResultModel(DEF_resultOut_grids, tr("Grilles combinées"));

    resultModel->setRootGroup(DEF_groupOut_grids, new CT_StandardItemGroup(), tr("Grilles"));
    resultModel->addItemModel(DEF_groupOut_grids, DEF_itemOut_hits, new CT_Grid3D<int>(), tr("hits"));
    resultModel->addItemModel(DEF_groupOut_grids, DEF_itemOut_theorical, new CT_Grid3D<int>(), tr("theorical"));
    resultModel->addItemModel(DEF_groupOut_grids, DEF_itemOut_before, new CT_Grid3D<int>(), tr("before"));
    resultModel->addItemModel(DEF_groupOut_grids, DEF_itemOut_density, new CT_Grid3D<double>(), tr("density - %1").arg(modeString));
    resultModel->addItemModel(DEF_groupOut_grids, DEF_itemOut_scanId, new CT_Grid3D<int>(), tr("scanId"));

    setMaximumTurn(1);
}

// Semi-automatic creation of step parameters DialogBox
void LVOX_StepCombineDensityGrids::createPostConfigurationDialog()
{
    CT_InTurnManager *man = getInTurnManager();
    man->getTurnIndexManager()->resetTurnIndex();
    man->createSearchModelListForCurrentTurn();

    // Gets in models to test the presence of optional grids
    CT_InResultModelGroup* resultIn_model = (CT_InResultModelGroup*) man->getInResultModel(DEF_resultIn_grids);
    CT_OutAbstractResultModel *resultOut_model = (CT_OutAbstractResultModel*) resultIn_model->getPossibilitiesSavedSelected().first()->outModel();

    //CT_ResultGroup* resultIn_grids = getInputResults().first();

    CT_InAbstractSingularItemModel *itemInModel_hits = (CT_InAbstractSingularItemModel*)PS_MODELS->searchModel(DEF_itemIn_hits, resultOut_model, this);
    CT_InAbstractSingularItemModel *itemInModel_theorical = (CT_InAbstractSingularItemModel*)PS_MODELS->searchModel(DEF_itemIn_theorical, resultOut_model, this);
    CT_InAbstractSingularItemModel *itemInModel_before = (CT_InAbstractSingularItemModel*)PS_MODELS->searchModel(DEF_itemIn_before, resultOut_model, this);

    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();


    CT_ButtonGroup &bg_mode = configDialog->addButtonGroup(_mode);

    configDialog->addText("Mode de combinaison", "des grilles de densité :","");
    configDialog->addExcludeValue("max (densité)", "", "", bg_mode, maxDensity);

    if (itemInModel_theorical->isAtLeastOnePossibilitySelected() && itemInModel_before->isAtLeastOnePossibilitySelected())
    {
        configDialog->addExcludeValue("max (nt - nb)", "", "", bg_mode, maxNt_Nb);
    } else {
        configDialog->addText("max (nt - nb)", "Non disponible : ", "grille(s) nt/nb manquante(s)");
    }

    if (itemInModel_hits->isAtLeastOnePossibilitySelected())
    {
        configDialog->addExcludeValue("max (ni)", "", "", bg_mode, maxNi);
    } else {
        configDialog->addText("max (ni)", "Non disponible : ", "grille ni manquante");
    }

    if (itemInModel_hits->isAtLeastOnePossibilitySelected() && itemInModel_theorical->isAtLeastOnePossibilitySelected() && itemInModel_before->isAtLeastOnePossibilitySelected())
    {
        configDialog->addExcludeValue("sum(ni) / sum(nt - nb)", "", "", bg_mode, sumNiSumNtNb);
        configDialog->addInt(tr("    -> (nt - nb) minimum"),tr(""),-100000,100000, _effectiveRayThresh );
    } else {
        configDialog->addText("sum(ni) / sum(nt - nb)", "Non disponible : ", "grille(s) ni/nt/nb manquante(s)");
    }

}

void LVOX_StepCombineDensityGrids::compute()
{
    CT_ResultGroup* resultIn_grids = getInputResults().first();

    CT_InAbstractSingularItemModel *itemInModel_hits = (CT_InAbstractSingularItemModel*)PS_MODELS->searchModel(DEF_itemIn_hits, resultIn_grids, this);
    CT_InAbstractSingularItemModel *itemInModel_theorical = (CT_InAbstractSingularItemModel*)PS_MODELS->searchModel(DEF_itemIn_theorical, resultIn_grids, this);
    CT_InAbstractSingularItemModel *itemInModel_before = (CT_InAbstractSingularItemModel*)PS_MODELS->searchModel(DEF_itemIn_before, resultIn_grids, this);

    bool use_ni = itemInModel_hits->isAtLeastOnePossibilitySelected();
    bool use_nt = itemInModel_theorical->isAtLeastOnePossibilitySelected();
    bool use_nb = itemInModel_before->isAtLeastOnePossibilitySelected();

    // coherence control between models and _mode parameter
    if (_mode < 0 || _mode > 2) {qDebug() << "Configuration non prévue !"; return;}
    if (_mode == maxNt_Nb && (!use_nt || !use_nb)) {qDebug() << "Configuration non prévue !"; return;}
    if (_mode == maxNi && !use_ni) {qDebug() << "Configuration non prévue !"; return;}
    if (_mode == sumNiSumNtNb && (!use_nt || !use_nb || !use_ni)) {qDebug() << "Configuration non prévue !"; return;}

    float xmin, ymin, zmin, res, NAd;
    size_t xdim, ydim, zdim;
    int NAi, NAt, NAb;
    bool firstGrid = true;

    QVector<const CT_Grid3D<int>*> InGrids_hits;
    QVector<const CT_Grid3D<int>*> InGrids_theorical;
    QVector<const CT_Grid3D<int>*> InGrids_before;
    QVector<const CT_Grid3D<double>*> InGrids_density;

    // Iterating on groups situated at the root of the result (corresponding to DEF_groupIn_grids)
    // => Create grids lists, and retrieve grids characteristics
    CT_ResultGroupIterator itGrp(resultIn_grids, this, DEF_groupIn_grids);
    while (itGrp.hasNext() && !isStopped())
    {
        CT_AbstractItemGroup *groupIn_grids = (CT_AbstractItemGroup*) itGrp.next();

        const CT_Grid3D<int>* itemIn_hits;
        const CT_Grid3D<int>* itemIn_theorical;
        const CT_Grid3D<int>* itemIn_before;
        const CT_Grid3D<double>* itemIn_density;

        if (use_ni) {itemIn_hits = (const CT_Grid3D<int>*) groupIn_grids->firstItemByINModelName(this, DEF_itemOut_hits);}
        if (use_nt) {itemIn_theorical = (const CT_Grid3D<int>*) groupIn_grids->firstItemByINModelName(this, DEF_itemIn_theorical);}
        if (use_nb) {itemIn_before = (const CT_Grid3D<int>*) groupIn_grids->firstItemByINModelName(this, DEF_itemIn_before);}
        itemIn_density = (const CT_Grid3D<double>*) groupIn_grids->firstItemByINModelName(this, DEF_itemIn_density);

        if (itemIn_density!=NULL)
        {
            if (use_ni && itemIn_hits==NULL) {qDebug() << "Grille ni manquante !";return; }
            if (use_nt && itemIn_theorical==NULL) {qDebug() << "Grille nt manquante !";return; }
            if (use_nb && itemIn_before==NULL) {qDebug() << "Grille nb manquante !";return; }

            if (firstGrid)
            {
                firstGrid = false;
                xmin = itemIn_density->minX();
                ymin = itemIn_density->minY();
                zmin = itemIn_density->minZ();
                xdim = itemIn_density->xdim();
                ydim = itemIn_density->ydim();
                zdim = itemIn_density->zdim();
                res = itemIn_density->resolution();
                NAd = itemIn_density->NA();
                NAi = itemIn_hits->NA();
                NAt = itemIn_theorical->NA();
                NAb = itemIn_before->NA();
            } else {
                if (xmin != itemIn_density->minX()) {qDebug() << "xmin non homogène !"; return; }
                if (ymin != itemIn_density->minY()) {qDebug() << "ymin non homogène !"; return; }
                if (zmin != itemIn_density->minZ()) {qDebug() << "zmin non homogène !"; return; }
                if (xdim != itemIn_density->xdim()) {qDebug() << "xdim non homogène !"; return; }
                if (ydim != itemIn_density->ydim()) {qDebug() << "ydim non homogène !"; return; }
                if (zdim != itemIn_density->zdim()) {qDebug() << "zdim non homogène !"; return; }
                if (res != itemIn_density->resolution()) {qDebug() << "resolution non homogène !"; return; }
                if (NAd != itemIn_density->NA()) {qDebug() << "NAd non homogène !"; return; }
                if (NAi != itemIn_hits->NA()) {qDebug() << "NAi non homogène !"; return; }
                if (NAt != itemIn_theorical->NA()) {qDebug() << "NAt non homogène !"; return; }
                if (NAb != itemIn_before->NA()) {qDebug() << "NAb non homogène !"; return; }
            }

            if (use_ni) {InGrids_hits.append(itemIn_hits);}
            if (use_nt) {InGrids_theorical.append(itemIn_theorical);}
            if (use_nb) {InGrids_before.append(itemIn_before);}
            InGrids_density.append(itemIn_density);
        } else
        {
            qDebug() << "Grille de densité manquante ! Le traitement continue avec les autres.";
        }
    }

    if (InGrids_density.size() <=0) {qDebug() << "Aucune Grille !"; return;}

    // Create combined out grids, considering optional input grids
    QList<CT_ResultGroup*> outResultList = getOutResultList();
    CT_ResultGroup* resultOut_grids = outResultList.at(0);

    CT_StandardItemGroup* groupOut_grids = new CT_StandardItemGroup(DEF_groupOut_grids, resultOut_grids);

    CT_Grid3D<int>* itemOut_hits = NULL;
    if (use_ni)
    {
        itemOut_hits = new CT_Grid3D<int>(DEF_itemOut_hits, resultOut_grids, xmin, ymin, zmin, xdim, ydim, zdim, res, NAi, NAi);
        groupOut_grids->addItemDrawable(itemOut_hits);
    }

    CT_Grid3D<int>* itemOut_theorical = NULL;
    if (use_nt)
    {
        itemOut_theorical = new CT_Grid3D<int>(DEF_itemOut_theorical, resultOut_grids, xmin, ymin, zmin, xdim, ydim, zdim, res, NAt, NAt);
        groupOut_grids->addItemDrawable(itemOut_theorical);
    }

    CT_Grid3D<int>* itemOut_before = NULL;
    if (use_nb)
    {
        itemOut_before = new CT_Grid3D<int>(DEF_itemOut_before, resultOut_grids, xmin, ymin, zmin, xdim, ydim, zdim, res, NAb, NAb);
        groupOut_grids->addItemDrawable(itemOut_before);
    }

    CT_Grid3D<double>* itemOut_density = new CT_Grid3D<double>(DEF_itemOut_density, resultOut_grids, xmin, ymin, zmin, xdim, ydim, zdim, res, NAd, NAd);
    groupOut_grids->addItemDrawable(itemOut_density);

    CT_Grid3D<int>* itemOut_scanId = new CT_Grid3D<int>(DEF_itemOut_scanId, resultOut_grids, xmin, ymin, zmin, xdim, ydim, zdim, res, -1, -1);
    groupOut_grids->addItemDrawable(itemOut_scanId);

    resultOut_grids->addGroup(groupOut_grids);


    const CT_Grid3D<double>* in_d = NULL;
    const CT_Grid3D<int>* in_ni = NULL;
    const CT_Grid3D<int>* in_nt = NULL;
    const CT_Grid3D<int>* in_nb = NULL;

    // Compute combined grids values
    // Init with first grids
    in_d = InGrids_density.at(0);
    if (use_ni) {in_ni = InGrids_hits.at(0);}
    if (use_nt) {in_nt = InGrids_theorical.at(0);}
    if (use_nb) {in_nb = InGrids_before.at(0);}

    for (size_t xx = 0 ; xx < xdim ; xx++)
    {
        for (size_t yy = 0 ; yy < ydim ; yy++)
        {
            for (size_t zz = 0 ; zz < zdim ; zz++)
            {
                size_t index;
                in_d->index(xx, yy, zz, index);
                itemOut_density->setValueAtIndex(index, in_d->valueAtIndex(index));
                if (use_ni) {itemOut_hits->setValueAtIndex(index, in_ni->valueAtIndex(index));}
                if (use_nt) {itemOut_theorical->setValueAtIndex(index, in_nt->valueAtIndex(index));}
                if (use_nb) {itemOut_before->setValueAtIndex(index, in_nb->valueAtIndex(index));}
            }
        }
    }

    // Compare with others grids
    for (int i = 1 ; i < InGrids_density.size() ; i++)
    {
        in_d = InGrids_density.at(i);
        if (use_ni) {in_ni = InGrids_hits.at(i);}
        if (use_nt) {in_nt = InGrids_theorical.at(i);}
        if (use_nb) {in_nb = InGrids_before.at(i);}

        for (size_t xx = 0 ; xx < xdim ; xx++)
        {
            for (size_t yy = 0 ; yy < ydim ; yy++)
            {
                for (size_t zz = 0 ; zz < zdim ; zz++)
                {
                    size_t index;
                    in_d->index(xx, yy, zz, index);
                    bool replace = false;

                    if (_mode == maxDensity) {
                        replace = (in_d->valueAtIndex(index) > itemOut_density->valueAtIndex(index));
                    } else if (_mode == maxNt_Nb) {
                        replace = ((in_nt->valueAtIndex(index) - in_nb->valueAtIndex(index)) > (itemOut_theorical->valueAtIndex(index) - itemOut_before->valueAtIndex(index)));
                    } else if (_mode == maxNi) {
                        replace = (in_ni->valueAtIndex(index) > itemOut_hits->valueAtIndex(index));
                    } else if (_mode == sumNiSumNtNb) {
                        replace = false;

                        itemOut_hits->addValueAtIndex(index, in_ni->valueAtIndex(index));
                        itemOut_theorical->addValueAtIndex(index, in_nt->valueAtIndex(index));
                        itemOut_before->addValueAtIndex(index, in_nb->valueAtIndex(index));

                        if (i == (InGrids_density.size() - 1))
                        {                            
                            int ni = itemOut_hits->valueAtIndex(index);
                            int nt = itemOut_theorical->valueAtIndex(index);
                            int nb = itemOut_before->valueAtIndex(index);

                            int ntMnb = nt - nb;

                            // Avoid division by 0
                            if (ntMnb == 0 )
                            {
                                itemOut_density->setValueAtIndex(index, -1);
                            }
                            // If there is an error (nb > nt)
                            else if (ntMnb < 0 )
                            {
                                itemOut_density->setValueAtIndex(index, -2);
                            }
                            // If there is not enough information
                            else if (ntMnb < _effectiveRayThresh )
                            {
                                itemOut_density->setValueAtIndex(index, -3);
                            }
                            // Normal case
                            else
                            {
                                float density = (float) ni / ((float) ntMnb);
                                itemOut_density->setValueAtIndex(index, density);
                            }
                        }
                    }

                    if (replace)
                    {
                        itemOut_density->setValueAtIndex(index, in_d->valueAtIndex(index));
                        if (use_ni) {itemOut_hits->setValueAtIndex(index, in_ni->valueAtIndex(index));}
                        if (use_nt) {itemOut_theorical->setValueAtIndex(index, in_nt->valueAtIndex(index));}
                        if (use_nb) {itemOut_before->setValueAtIndex(index, in_nb->valueAtIndex(index));}
                    }
                }
            }
        }
    }

    if (itemOut_hits!=NULL) {itemOut_hits->computeMinMax();}
    if (itemOut_theorical!=NULL) {itemOut_theorical->computeMinMax();}
    if (itemOut_before!=NULL) {itemOut_before->computeMinMax();}
    if (itemOut_density!=NULL) {itemOut_density->computeMinMax();}

}
