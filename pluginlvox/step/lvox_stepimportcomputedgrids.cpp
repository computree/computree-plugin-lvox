#include "lvox_stepimportcomputedgrids.h"

#include "ct_itemdrawable/ct_grid3d.h"
#include "ct_itemdrawable/tools/iterator/ct_groupiterator.h"
#include "ct_result/ct_resultgroup.h"
#include "ct_result/model/outModel/ct_outresultmodelgroup.h"
#include "ct_view/ct_stepconfigurabledialog.h"
#include "ct_model/tools/ct_modelsearchhelper.h"
#include "ctlibio/readers/ct_reader_asciigrid3d.h"
#include "ct_itemdrawable/ct_standarditemgroup.h"
#include <qdir.h>

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
LVOX_StepImportComputedGrids::LVOX_StepImportComputedGrids(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
}

// Step description (tooltip of contextual menu)
QString LVOX_StepImportComputedGrids::getStepDescription() const
{
    return tr("2b - Import computed grids into one result");
}

// Step detailled description
QString LVOX_StepImportComputedGrids::getStepDetailledDescription() const
{
    return tr("Import grids computed by step 2 of LVOX into one result");
}

// Step URL
QString LVOX_StepImportComputedGrids::getStepURL() const
{
    //return tr("STEP URL HERE");
    return CT_AbstractStep::getStepURL(); //by default URL of the plugin
}

// Step copy method
CT_VirtualAbstractStep* LVOX_StepImportComputedGrids::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new LVOX_StepImportComputedGrids(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void LVOX_StepImportComputedGrids::createInResultModelListProtected()
{
    setNotNeedInputResult();
}

// Creation and affiliation of OUT models
void LVOX_StepImportComputedGrids::createOutResultModelListProtected()
{
    CT_OutResultModelGroup* out_res = createNewOutResultModel(DEF_out_res, tr("Result"));

    if (out_res)
    {
        out_res->setRootGroup(DEF_out_grp, new CT_StandardItemGroup(), tr("Grilles"));

        out_res->addItemModel(DEF_out_grp, DEF_hits, new CT_Grid3D<int>, tr("Hits"));
        out_res->addItemAttributeModel(DEF_hits, DEF_attr_ni, new CT_StdItemAttributeT<bool>("LVOX_GRD_NI"), tr("isNi"));

        out_res->addItemModel(DEF_out_grp, DEF_theo, new CT_Grid3D<int>, tr("Theoretical"));
        out_res->addItemAttributeModel(DEF_theo, DEF_attr_nt, new CT_StdItemAttributeT<bool>("LVOX_GRD_NT"), tr("isNt"));

        out_res->addItemModel(DEF_out_grp, DEF_before, new CT_Grid3D<int>, tr("Before"));
        out_res->addItemAttributeModel(DEF_before, DEF_attr_nb, new CT_StdItemAttributeT<bool>("LVOX_GRD_NB"), tr("isNb"));

        out_res->addItemModel(DEF_out_grp, DEF_density, new CT_Grid3D<float>, tr("Density"));
        out_res->addItemAttributeModel(DEF_density, DEF_attr_density, new CT_StdItemAttributeT<bool>("LVOX_GRD_DENSITY"), tr("isDensity"));
    }
}

// Semi-automatic creation of step parameters DialogBox
void LVOX_StepImportComputedGrids::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog* diag = newStandardPostConfigurationDialog();

    diag->addFileChoice(tr("Choisir le dossier contenant les grilles"), CT_FileChoiceButton::OneExistingFolder, "", _folder);
}

template<class T>
CT_Grid3D<T>* readFile(QString filename, CT_OutAbstractSingularItemModel* model, CT_ResultGroup* result)
{
    CT_Reader_AsciiGrid3D reader{std::is_same<T, float>::value};
    CT_Grid3D<T>* grid;

    if(reader.setFilePath(filename))
    {
        reader.init();
        if(reader.readFile())
        {
            grid = (CT_Grid3D<T>*) reader.takeFirstItemDrawableOfType(CT_Grid3D<T>::staticGetType(), result, model);
            grid->computeMinMax();
        }
    }
    return grid;
}

void LVOX_StepImportComputedGrids::compute()
{
    CT_ResultGroup* out_res = getOutResultList().at(0);
    CT_StandardItemGroup* out_group = new CT_StandardItemGroup(DEF_out_grp, out_res);

    CT_OutAbstractSingularItemModel* hits = (CT_OutAbstractSingularItemModel*) PS_MODELS->searchModel(DEF_hits, out_res, this);
    CT_OutAbstractSingularItemModel* theo = (CT_OutAbstractSingularItemModel*) PS_MODELS->searchModel(DEF_theo, out_res, this);
    CT_OutAbstractSingularItemModel* before = (CT_OutAbstractSingularItemModel*) PS_MODELS->searchModel(DEF_before, out_res, this);
    CT_OutAbstractSingularItemModel* density = (CT_OutAbstractSingularItemModel*) PS_MODELS->searchModel(DEF_density, out_res, this);

    QDir dir(_folder.first());
    if(dir.exists())
    {
        int n = dir.count()-2;
        QStringList hits_list = dir.entryList(QStringList()<<"ni*", QDir::Files, QDir::Name);
        QStringList theo_list = dir.entryList(QStringList()<<"nt*", QDir::Files, QDir::Name);
        QStringList before_list = dir.entryList(QStringList()<<"nb*", QDir::Files, QDir::Name);
        QStringList density_list = dir.entryList(QStringList()<<"density*", QDir::Files, QDir::Name);

        for(int i = 0; i < n/4; i++)
        {
            CT_Grid3D<int>* hits_grd = readFile<int>(hits_list[i], hits, out_res);
            out_group->addItemDrawable(hits_grd);
            CT_Grid3D<int>* theo_grd = readFile<int>(theo_list[i], theo, out_res);
            out_group->addItemDrawable(theo_grd);
            CT_Grid3D<int>* before_grd = readFile<int>(before_list[i], before, out_res);
            out_group->addItemDrawable(before_grd);
            CT_Grid3D<int>* density_grd = readFile<int>(density_list[i], density, out_res);
            out_group->addItemDrawable(density_grd);
        }
    }
    else
    {
        PS_LOG->addMessage(LogInterface::trace, LogInterface::step, QObject::tr("Invalid directory"));
    }

//    CT_Grid3D<int>* hits_grd = readFile<int>(_hits.first(), hits, out_res);
//    out_group->addItemDrawable(hits_grd);
//    setProgress(25);

//    CT_Grid3D<int>* theo_grd = readFile<int>(_theo.first(), theo, out_res);
//    out_group->addItemDrawable(theo_grd);
//    setProgress(50);

//    CT_Grid3D<int>* before_grd = readFile<int>(_before.first(), before, out_res);
//    out_group->addItemDrawable(before_grd);
//    setProgress(75);

//    CT_Grid3D<float>* density_grd = readFile<float>(_density.first(), density, out_res);
//    out_group->addItemDrawable(density_grd);
//    setProgress(100);

    out_res->addGroup(out_group);
}
