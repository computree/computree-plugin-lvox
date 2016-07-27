#include "lvox_stepmergeinputs.h"

#include "ct_itemdrawable/ct_grid3d.h"
#include "ct_itemdrawable/tools/iterator/ct_groupiterator.h"
#include "ct_result/ct_resultgroup.h"
#include "ct_result/model/outModel/ct_outresultmodelgroup.h"
#include "ct_view/ct_stepconfigurabledialog.h"
#include "ct_model/tools/ct_modelsearchhelper.h"
#include "ctlibio/readers/ct_reader_asciigrid3d.h"
#include "ct_itemdrawable/ct_standarditemgroup.h"

// Alias for indexing models
#define DEF_out_res "result"
#define DEF_out_grp "group"
#define DEF_hits "hits"
#define DEF_theo "theoretical"
#define DEF_before "before"
#define DEF_density "density"
#define DEF_attr_ni "isNi"
#define DEF_attr_nt "isNt"
#define DEF_attr_nb "isNb"
#define DEF_attr_density "isDensity"

// Constructor : initialization of parameters
LVOX_StepMergeInputs::LVOX_StepMergeInputs(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
}

// Step description (tooltip of contextual menu)
QString LVOX_StepMergeInputs::getStepDescription() const
{
    return tr("0 - Merge exported grids into one result");
}

// Step detailled description
QString LVOX_StepMergeInputs::getStepDetailledDescription() const
{
    return tr("Merge exported grids into one result");
}

// Step URL
QString LVOX_StepMergeInputs::getStepURL() const
{
    //return tr("STEP URL HERE");
    return CT_AbstractStep::getStepURL(); //by default URL of the plugin
}

// Step copy method
CT_VirtualAbstractStep* LVOX_StepMergeInputs::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new LVOX_StepMergeInputs(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void LVOX_StepMergeInputs::createInResultModelListProtected()
{
    setNotNeedInputResult();
}

// Creation and affiliation of OUT models
void LVOX_StepMergeInputs::createOutResultModelListProtected()
{
    CT_OutResultModelGroup* out_res = createNewOutResultModel(DEF_out_res, tr("Result"));

    if (out_res)
    {
        out_res->setRootGroup(DEF_out_grp, new CT_StandardItemGroup(), tr("Grilles"));

        out_res->addItemModel(DEF_out_grp, DEF_hits, new CT_Grid3D<float>, tr("Hits"));
        out_res->addItemAttributeModel(DEF_hits, DEF_attr_ni, new CT_StdItemAttributeT<bool>("LVOX_GRD_NI"), tr("isNi"));

        out_res->addItemModel(DEF_out_grp, DEF_theo, new CT_Grid3D<float>, tr("Theoretical"));
        out_res->addItemAttributeModel(DEF_theo, DEF_attr_nt, new CT_StdItemAttributeT<bool>("LVOX_GRD_NT"), tr("isNt"));

        out_res->addItemModel(DEF_out_grp, DEF_before, new CT_Grid3D<float>, tr("Before"));
        out_res->addItemAttributeModel(DEF_before, DEF_attr_nb, new CT_StdItemAttributeT<bool>("LVOX_GRD_NB"), tr("isNb"));

        out_res->addItemModel(DEF_out_grp, DEF_density, new CT_Grid3D<float>, tr("Density"));
        out_res->addItemAttributeModel(DEF_density, DEF_attr_density, new CT_StdItemAttributeT<bool>("LVOX_GRD_DENSITY"), tr("isDensity"));
    }
}

// Semi-automatic creation of step parameters DialogBox
void LVOX_StepMergeInputs::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog* diag = newStandardPostConfigurationDialog();

    diag->addFileChoice(tr("Choisir le fichier de la grille 'hits'"), CT_FileChoiceButton::OneExistingFile, "Fichier 'hits' (*.*)", _hits);
    diag->addFileChoice(tr("Choisir le fichier de la grille 'theoretical'"), CT_FileChoiceButton::OneExistingFile, "Fichier 'theo' (*.*)", _theo);
    diag->addFileChoice(tr("Choisir le fichier de la grille 'before'"), CT_FileChoiceButton::OneExistingFile, "Fichier 'before' (*.*)", _before);
    diag->addFileChoice(tr("Choisir le fichier de la grille 'intensity'"), CT_FileChoiceButton::OneExistingFile, "Fichier 'intensity' (*.*)", _density);
}

CT_Grid3D<float>* readFile(QString filename, CT_OutAbstractSingularItemModel* model, CT_ResultGroup* result)
{
    CT_Reader_AsciiGrid3D reader;
    CT_Grid3D<float>* grid;

    if(reader.setFilePath(filename))
    {
        reader.init();
        if(reader.readFile())
            grid = (CT_Grid3D<float>*) reader.takeFirstItemDrawableOfType(CT_Grid3D<float>::staticGetType(), result, model);
    }
    grid->computeMinMax();
    return grid;
}

void LVOX_StepMergeInputs::compute()
{
    CT_ResultGroup* out_res = getOutResultList().at(0);
    CT_StandardItemGroup* out_group = new CT_StandardItemGroup(DEF_out_grp, out_res);

    CT_OutAbstractSingularItemModel* hits      = (CT_OutAbstractSingularItemModel*) PS_MODELS->searchModel(DEF_hits, out_res, this);
    CT_OutAbstractSingularItemModel* theo      = (CT_OutAbstractSingularItemModel*) PS_MODELS->searchModel(DEF_theo, out_res, this);
    CT_OutAbstractSingularItemModel* before    = (CT_OutAbstractSingularItemModel*) PS_MODELS->searchModel(DEF_before, out_res, this);
    CT_OutAbstractSingularItemModel* density = (CT_OutAbstractSingularItemModel*) PS_MODELS->searchModel(DEF_density, out_res, this);

    CT_Grid3D<float>* hits_grd = readFile(_hits.first(), hits, out_res);
    out_group->addItemDrawable(hits_grd);
    setProgress(25);

    CT_Grid3D<float>* theo_grd = readFile(_theo.first(), theo, out_res);
    out_group->addItemDrawable(theo_grd);
    setProgress(50);

    CT_Grid3D<float>* before_grd = readFile(_before.first(), before, out_res);
    out_group->addItemDrawable(before_grd);
    setProgress(75);


    CT_Grid3D<float>* density_grd = readFile(_density.first(), density, out_res);
    out_group->addItemDrawable(density_grd);
    setProgress(100);

    out_res->addGroup(out_group);
}
