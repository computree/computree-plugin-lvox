#include "lvox_stepcombinedensitygrids.h"

// Inclusion of in models
#include "ct_itemdrawable/model/inModel/ct_instandardgroupmodel.h"
#include "ct_itemdrawable/model/inModel/ct_instandarditemdrawablemodel.h"
#include "ct_result/model/inModel/ct_inresultmodelgroup.h"

// Inclusion of out models
#include "ct_itemdrawable/model/outModel/ct_outstandardgroupmodel.h"
#include "ct_itemdrawable/model/outModel/ct_outstandardgroupmodel.h"
#include "ct_itemdrawable/model/outModel/ct_outstandarditemdrawablemodel.h"
#include "ct_result/model/outModel/ct_outresultmodelgroup.h"

// Inclusion of standard result class
#include "ct_result/ct_resultgroup.h"

// Inclusion of used ItemDrawable classes
#include "ct_itemdrawable/ct_grid3d.h"
#include "qdebug.h"

#include "ct_result/model/inModel/tools/ct_inturnmanager.h"

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

    CT_InStandardGroupModel *groupInModel_grids = new CT_InStandardGroupModel(DEF_groupIn_grids,
                                                                              "Group");

    CT_InStandardItemDrawableModel *itemInModel_hits = new CT_InStandardItemDrawableModel(DEF_itemIn_hits,
                                                                                          CT_Grid3D<int>::staticGetType(),
                                                                                          tr("hits"),
                                                                                          tr(""),
                                                                                          CT_InStandardItemDrawableModel::C_ChooseOneIfMultiple,
                                                                                          CT_InStandardItemDrawableModel::F_IsOptional);

    CT_InStandardItemDrawableModel *itemInModel_theorical = new CT_InStandardItemDrawableModel(DEF_itemIn_theorical,
                                                                                               CT_Grid3D<int>::staticGetType(),
                                                                                               tr("theorical"),
                                                                                               tr(""),
                                                                                               CT_InStandardItemDrawableModel::C_ChooseOneIfMultiple,
                                                                                               CT_InStandardItemDrawableModel::F_IsOptional);

    CT_InStandardItemDrawableModel *itemInModel_before = new CT_InStandardItemDrawableModel(DEF_itemIn_before,
                                                                                            CT_Grid3D<int>::staticGetType(),
                                                                                            tr("before"),
                                                                                            tr(""),
                                                                                            CT_InStandardItemDrawableModel::C_ChooseOneIfMultiple,
                                                                                            CT_InStandardItemDrawableModel::F_IsOptional);

    CT_InStandardItemDrawableModel *itemInModel_density = new CT_InStandardItemDrawableModel(DEF_itemIn_density,
                                                                                             CT_Grid3D<double>::staticGetType(),
                                                                                             tr("density"));

    groupInModel_grids->addItem(itemInModel_hits);
    groupInModel_grids->addItem(itemInModel_theorical);
    groupInModel_grids->addItem(itemInModel_before);
    groupInModel_grids->addItem(itemInModel_density);


    CT_InResultModelGroup *resultInModel_grids = new CT_InResultModelGroup(DEF_resultIn_grids,
                                                                           groupInModel_grids,
                                                                           tr(""),
                                                                           tr(""),
                                                                           false);

    addInResultModel(resultInModel_grids);
}

// Creation and affiliation of OUT models
void LVOX_StepCombineDensityGrids::createOutResultModelListProtected()
{
    QString modeString;
    if (_mode == maxDensity)    {modeString = "max(density)";}
    else if (_mode == maxNt_Nb) {modeString = "max(nt-nb)";}
    else if (_mode == maxNi)    {modeString = "max(ni)";}

    CT_OutStandardGroupModel *groupOutModel_grids = new CT_OutStandardGroupModel(DEF_groupOut_grids,
                                                                                 new CT_StandardItemGroup(),
                                                                                 tr("grids"));

    CT_OutStandardItemDrawableModel *itemOutModel_hits = new CT_OutStandardItemDrawableModel(DEF_itemOut_hits,
                                                                                             new CT_Grid3D<int>,
                                                                                             tr("hits"));

    CT_OutStandardItemDrawableModel *itemOutModel_theorical = new CT_OutStandardItemDrawableModel(DEF_itemOut_theorical,
                                                                                                  new CT_Grid3D<int>,
                                                                                                  tr("theorical"));

    CT_OutStandardItemDrawableModel *itemOutModel_before = new CT_OutStandardItemDrawableModel(DEF_itemOut_before,
                                                                                               new CT_Grid3D<int>,
                                                                                               tr("before"));

    CT_OutStandardItemDrawableModel *itemOutModel_density = new CT_OutStandardItemDrawableModel(DEF_itemOut_density,
                                                                                                new CT_Grid3D<double>,
                                                                                                tr("density - %1").arg(modeString));

    CT_OutStandardItemDrawableModel *itemOutModel_scanId = new CT_OutStandardItemDrawableModel(DEF_itemOut_scanId,
                                                                                               new CT_Grid3D<int>,
                                                                                               tr("scanId"));

    groupOutModel_grids->addItem(itemOutModel_hits);
    groupOutModel_grids->addItem(itemOutModel_theorical);
    groupOutModel_grids->addItem(itemOutModel_before);
    groupOutModel_grids->addItem(itemOutModel_density);
    groupOutModel_grids->addItem(itemOutModel_scanId);

    CT_OutResultModelGroup *resultOutModel_grids = new CT_OutResultModelGroup(DEF_resultOut_grids,
                                                                              groupOutModel_grids,
                                                                              tr("grids"),
                                                                              tr(""));
    addOutResultModel(resultOutModel_grids);
}

// Semi-automatic creation of step parameters DialogBox
void LVOX_StepCombineDensityGrids::createPostConfigurationDialog()
{
    CT_InTurnManager *man = getInTurnManager();
    man->getTurnIndexManager()->resetTurnIndex();
    man->createSearchModelListForCurrentTurn();

    // Gets in models to test the presence of optional grids
    CT_InResultModelGroup* resultIn_model = (CT_InResultModelGroup*) man->getInResultModel(DEF_resultIn_grids);
    CT_OutAbstractResultModel *resultOut_model = resultIn_model->getPossibilitiesSavedChecked().first()->outModel();

    CT_InAbstractItemDrawableModel* itemInModel_hits = (CT_InAbstractItemDrawableModel*)getInModelForResearch(resultOut_model, DEF_itemIn_hits);
    CT_InAbstractItemDrawableModel* itemInModel_theorical = (CT_InAbstractItemDrawableModel*)getInModelForResearch(resultOut_model, DEF_itemIn_theorical);
    CT_InAbstractItemDrawableModel* itemInModel_before = (CT_InAbstractItemDrawableModel*)getInModelForResearch(resultOut_model, DEF_itemIn_before);

    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    CT_ButtonGroup &bg_mode = configDialog->addButtonGroup(_mode);

    configDialog->addText("Mode de combinaison des grilles de densité :","","");
    configDialog->addExcludeValue("", "", "max (densité)", bg_mode, maxDensity);

    if (itemInModel_theorical->getPossibilitiesSavedChecked().size() > 0 && itemInModel_before->getPossibilitiesSavedChecked().size() > 0)
    {
        configDialog->addExcludeValue("", "", "max (nt - nb)", bg_mode, maxNt_Nb);
    } else {
        configDialog->addText("", "max (nt - nb)", "- Non disponible : grilles nt/nb manquante(s)");
    }

    if (itemInModel_hits->getPossibilitiesSavedChecked().size() > 0)
    {
        configDialog->addExcludeValue("", "", "max (ni)", bg_mode, maxNi);
    } else {
        configDialog->addText("", "max (ni)", "- Non disponible : grille ni manquante");
    }

}

void LVOX_StepCombineDensityGrids::compute()
{
    CT_ResultGroup* resultIn_grids = getInputResultsForModel(DEF_resultIn_grids).first();

    CT_InAbstractGroupModel* groupInModel_grids = (CT_InAbstractGroupModel*)getInModelForResearch(resultIn_grids, DEF_groupIn_grids);
    CT_InAbstractItemDrawableModel* itemInModel_hits = (CT_InAbstractItemDrawableModel*)getInModelForResearch(resultIn_grids, DEF_itemIn_hits);
    CT_InAbstractItemDrawableModel* itemInModel_theorical = (CT_InAbstractItemDrawableModel*)getInModelForResearch(resultIn_grids, DEF_itemIn_theorical);
    CT_InAbstractItemDrawableModel* itemInModel_before = (CT_InAbstractItemDrawableModel*)getInModelForResearch(resultIn_grids, DEF_itemIn_before);
    CT_InAbstractItemDrawableModel* itemInModel_density = (CT_InAbstractItemDrawableModel*)getInModelForResearch(resultIn_grids, DEF_itemIn_density);

    bool use_ni = (itemInModel_hits!=NULL);
    bool use_nt = (itemInModel_theorical!=NULL);
    bool use_nb = (itemInModel_before!=NULL);

    // coherence control between models and _mode parameter
    if (_mode < 0 || _mode > 2) {qDebug() << "Configuration non prévue !"; return;}
    if (_mode == maxNt_Nb && (!use_nt || !use_nb)) {qDebug() << "Configuration non prévue !"; return;}
    if (_mode == maxNi && !use_ni) {qDebug() << "Configuration non prévue !"; return;}

    float xmin, ymin, zmin, res, NAd;
    int xdim, ydim, zdim, NAi, NAt, NAb;
    bool firstGrid = true;

    QVector<const CT_Grid3D<int>*> InGrids_hits;
    QVector<const CT_Grid3D<int>*> InGrids_theorical;
    QVector<const CT_Grid3D<int>*> InGrids_before;
    QVector<const CT_Grid3D<double>*> InGrids_density;

    // Iterating on groups situated at the root of the result (corresponding to DEF_groupIn_grids)
    // => Create grids lists, and retrieve grids characteristics
    for ( CT_AbstractItemGroup *groupIn_grids = resultIn_grids->beginGroup(groupInModel_grids)
          ; groupIn_grids != NULL  && !isStopped()
          ; groupIn_grids = resultIn_grids->nextGroup() )
    {
        const CT_Grid3D<int>* itemIn_hits;
        const CT_Grid3D<int>* itemIn_theorical;
        const CT_Grid3D<int>* itemIn_before;
        const CT_Grid3D<double>* itemIn_density;

        if (use_ni) {itemIn_hits = (const CT_Grid3D<int>*) groupIn_grids->findFirstItem(itemInModel_hits);}
        if (use_nt) {itemIn_theorical = (const CT_Grid3D<int>*) groupIn_grids->findFirstItem(itemInModel_theorical);}
        if (use_nb) {itemIn_before = (const CT_Grid3D<int>*) groupIn_grids->findFirstItem(itemInModel_before);}
        itemIn_density = (const CT_Grid3D<double>*) groupIn_grids->findFirstItem(itemInModel_density);

        if (itemIn_density!=NULL)
        {
            if (use_ni && itemIn_hits==NULL) {qDebug() << "Grille ni manquante !";return; }
            if (use_nt && itemIn_theorical==NULL) {qDebug() << "Grille nt manquante !";return; }
            if (use_nb && itemIn_before==NULL) {qDebug() << "Grille nb manquante !";return; }

            if (firstGrid)
            {
                firstGrid = false;
                xmin = itemIn_density->xMin();
                ymin = itemIn_density->yMin();
                zmin = itemIn_density->zMin();
                xdim = itemIn_density->xdim();
                ydim = itemIn_density->ydim();
                zdim = itemIn_density->zdim();
                res = itemIn_density->resolution();
                NAd = itemIn_density->NA();
                NAi = itemIn_hits->NA();
                NAt = itemIn_theorical->NA();
                NAb = itemIn_before->NA();
            } else {
                if (xmin != itemIn_density->xMin()) {qDebug() << "xmin non homogène !"; return; }
                if (ymin != itemIn_density->yMin()) {qDebug() << "ymin non homogène !"; return; }
                if (zmin != itemIn_density->zMin()) {qDebug() << "zmin non homogène !"; return; }
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

    CT_OutStandardGroupModel* groupOutModel_grids = (CT_OutStandardGroupModel*)getOutModelForCreation(resultOut_grids, DEF_groupOut_grids);
    CT_StandardItemGroup* groupOut_grids = new CT_StandardItemGroup(groupOutModel_grids, resultOut_grids);

    CT_Grid3D<int>* itemOut_hits = NULL;
    if (use_ni)
    {
        CT_OutStandardItemDrawableModel* itemOutModel_hits = (CT_OutStandardItemDrawableModel*)getOutModelForCreation(resultOut_grids, DEF_itemOut_hits);
        itemOut_hits = new CT_Grid3D<int>(itemOutModel_hits, resultOut_grids, xmin, ymin, zmin, xdim, ydim, zdim, res, NAi, NAi);
        groupOut_grids->addItemDrawable(itemOut_hits);
    }

    CT_Grid3D<int>* itemOut_theorical = NULL;
    if (use_nt)
    {
        CT_OutStandardItemDrawableModel* itemOutModel_theorical = (CT_OutStandardItemDrawableModel*)getOutModelForCreation(resultOut_grids, DEF_itemOut_theorical);
        itemOut_theorical = new CT_Grid3D<int>(itemOutModel_theorical, resultOut_grids, xmin, ymin, zmin, xdim, ydim, zdim, res, NAt, NAt);
        groupOut_grids->addItemDrawable(itemOut_theorical);
    }

    CT_Grid3D<int>* itemOut_before = NULL;
    if (use_nb)
    {
        CT_OutStandardItemDrawableModel* itemOutModel_before = (CT_OutStandardItemDrawableModel*)getOutModelForCreation(resultOut_grids, DEF_itemOut_before);
        itemOut_before = new CT_Grid3D<int>(itemOutModel_before, resultOut_grids, xmin, ymin, zmin, xdim, ydim, zdim, res, NAb, NAb);
        groupOut_grids->addItemDrawable(itemOut_before);
    }

    CT_OutStandardItemDrawableModel* itemOutModel_density = (CT_OutStandardItemDrawableModel*)getOutModelForCreation(resultOut_grids, DEF_itemOut_density);
    CT_Grid3D<double>* itemOut_density = new CT_Grid3D<double>(itemOutModel_density, resultOut_grids, xmin, ymin, zmin, xdim, ydim, zdim, res, NAd, NAd);
    groupOut_grids->addItemDrawable(itemOut_density);

    CT_OutStandardItemDrawableModel* itemOutModel_scanId = (CT_OutStandardItemDrawableModel*)getOutModelForCreation(resultOut_grids, DEF_itemOut_scanId);
    CT_Grid3D<int>* itemOut_scanId = new CT_Grid3D<int>(itemOutModel_scanId, resultOut_grids, xmin, ymin, zmin, xdim, ydim, zdim, res, -1, -1);
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

    for (int xx = 0 ; xx < xdim ; xx++)
    {
        for (int yy = 0 ; yy < ydim ; yy++)
        {
            for (int zz = 0 ; zz < zdim ; zz++)
            {
                int index = in_d->index(xx, yy, zz);
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

        for (int xx = 0 ; xx < xdim ; xx++)
        {
            for (int yy = 0 ; yy < ydim ; yy++)
            {
                for (int zz = 0 ; zz < zdim ; zz++)
                {
                    int index = in_d->index(xx, yy, zz);
                    bool replace = false;

                    if (_mode == maxDensity) {
                        replace = (in_d->valueAtIndex(index) > itemOut_density->valueAtIndex(index));
                    } else if (_mode == maxNt_Nb) {
                        replace = ((in_nt->valueAtIndex(index) - in_nb->valueAtIndex(index)) > (itemOut_theorical->valueAtIndex(index) - itemOut_before->valueAtIndex(index)));
                    } else if (_mode = maxNi) {
                        replace = (in_ni->valueAtIndex(index) > itemOut_hits->valueAtIndex(index));
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

}
