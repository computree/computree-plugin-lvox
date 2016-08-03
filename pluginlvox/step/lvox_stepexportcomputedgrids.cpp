#include "lvox_stepexportcomputedgrids.h"

#include "ct_itemdrawable/ct_grid3d.h"
#include "ct_itemdrawable/tools/iterator/ct_groupiterator.h"
#include "ct_result/ct_resultgroup.h"
#include "ct_result/model/inModel/ct_inresultmodelgroup.h"
#include "ct_view/ct_stepconfigurabledialog.h"
#include "ct_model/tools/ct_modelsearchhelper.h"
#include "ctlibio/readers/ct_reader_asciigrid3d.h"
#include "ct_itemdrawable/ct_standarditemgroup.h"
#include "../pluginbase/exporters/grid3d/pb_grid3dexporter.h"

// Alias for indexing models
#define DEF_in_res "result"
#define DEF_in_grp "group"
#define DEF_hits "hits"
#define DEF_theo "theoretical"
#define DEF_before "before"
#define DEF_density "density"
#define DEF_attr_ni "isNi"
#define DEF_attr_nt "isNt"
#define DEF_attr_nb "isNb"
#define DEF_attr_density "isDensity"

// Constructor : initialization of parameters
LVOX_StepExportComputedGrids::LVOX_StepExportComputedGrids(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
}

// Step description (tooltip of contextual menu)
QString LVOX_StepExportComputedGrids::getStepDescription() const
{
    return tr("2.25 - Export computed grids");
}

// Step detailled description
QString LVOX_StepExportComputedGrids::getStepDetailledDescription() const
{
    return tr("Export all the computed grids into a folder");
}

// Step URL
QString LVOX_StepExportComputedGrids::getStepURL() const
{
    //return tr("STEP URL HERE");
    return CT_AbstractStep::getStepURL(); //by default URL of the plugin
}

// Step copy method
CT_VirtualAbstractStep* LVOX_StepExportComputedGrids::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new LVOX_StepExportComputedGrids(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void LVOX_StepExportComputedGrids::createInResultModelListProtected()
{
    CT_InResultModelGroup* in_res = createNewInResultModel(DEF_in_res, tr("Grilles"));

    in_res->setZeroOrMoreRootGroup();
    in_res->addGroupModel("", DEF_in_grp);

    in_res->addItemModel(DEF_in_grp, DEF_hits, CT_Grid3D<int>::staticGetType(), tr("hits"), "",
                              CT_InAbstractModel::C_ChooseOneIfMultiple, CT_InAbstractModel::F_IsOptional);
    in_res->addItemAttributeModel(DEF_hits, DEF_attr_ni, QList<QString>() << "LVOX_GRD_NI", CT_AbstractCategory::ANY, tr("isNi"), "", CT_InAbstractModel::C_ChooseOneIfMultiple, CT_InAbstractModel::F_IsOptional);

    in_res->addItemModel(DEF_in_grp, DEF_theo, CT_Grid3D<int>::staticGetType(), tr("theoretical"), "",
                              CT_InAbstractModel::C_ChooseOneIfMultiple, CT_InAbstractModel::F_IsOptional);
    in_res->addItemAttributeModel(DEF_theo, DEF_attr_nt, QList<QString>() << "LVOX_GRD_NT", CT_AbstractCategory::ANY, tr("isNt"), "", CT_InAbstractModel::C_ChooseOneIfMultiple, CT_InAbstractModel::F_IsOptional);

    in_res->addItemModel(DEF_in_grp, DEF_before, CT_Grid3D<int>::staticGetType(), tr("before"), "",
                              CT_InAbstractModel::C_ChooseOneIfMultiple, CT_InAbstractModel::F_IsOptional);
    in_res->addItemAttributeModel(DEF_before, DEF_attr_nb, QList<QString>() << "LVOX_GRD_NB", CT_AbstractCategory::ANY, tr("isNb"), "", CT_InAbstractModel::C_ChooseOneIfMultiple, CT_InAbstractModel::F_IsOptional);

    in_res->addItemModel(DEF_in_grp, DEF_density, CT_Grid3D<float>::staticGetType(), tr("density"));
    in_res->addItemAttributeModel(DEF_density, DEF_attr_density, QList<QString>() << "LVOX_GRD_DENSITY", CT_AbstractCategory::ANY, tr("isDensity"), "", CT_InAbstractModel::C_ChooseOneIfMultiple, CT_InAbstractModel::F_IsOptional);
}

// Creation and affiliation of OUT models
void LVOX_StepExportComputedGrids::createOutResultModelListProtected()
{
//    CT_OutResultModelGroup* out_res = createNewOutResultModel(DEF_out_res, tr("Result"));

//    if (out_res)
//    {
//        out_res->setRootGroup(DEF_out_grp, new CT_StandardItemGroup(), tr("Grilles"));

//        out_res->addItemModel(DEF_out_grp, DEF_hits, new CT_Grid3D<int>, tr("Hits"));
//        out_res->addItemAttributeModel(DEF_hits, DEF_attr_ni, new CT_StdItemAttributeT<bool>("LVOX_GRD_NI"), tr("isNi"));

//        out_res->addItemModel(DEF_out_grp, DEF_theo, new CT_Grid3D<int>, tr("Theoretical"));
//        out_res->addItemAttributeModel(DEF_theo, DEF_attr_nt, new CT_StdItemAttributeT<bool>("LVOX_GRD_NT"), tr("isNt"));

//        out_res->addItemModel(DEF_out_grp, DEF_before, new CT_Grid3D<int>, tr("Before"));
//        out_res->addItemAttributeModel(DEF_before, DEF_attr_nb, new CT_StdItemAttributeT<bool>("LVOX_GRD_NB"), tr("isNb"));

//        out_res->addItemModel(DEF_out_grp, DEF_density, new CT_Grid3D<float>, tr("Density"));
//        out_res->addItemAttributeModel(DEF_density, DEF_attr_density, new CT_StdItemAttributeT<bool>("LVOX_GRD_DENSITY"), tr("isDensity"));
//    }
}

// Semi-automatic creation of step parameters DialogBox
void LVOX_StepExportComputedGrids::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog* diag = newStandardPostConfigurationDialog();

    diag->addFileChoice(tr("Choisir la destination"), CT_FileChoiceButton::OneNewFile, "", _folder, "Choisissez le dossier de destination à créer", "", "");
}

//void LVOX_StepExportComputedGrids::postConfigure()
//{
//    QDir().mkdir(_folder);
//}

//template<class T>
//CT_Grid3D<T>* readFile(QString filename, CT_OutAbstractSingularItemModel* model, CT_ResultGroup* result)
//{
//    CT_Reader_AsciiGrid3D reader{std::is_same<T, float>::value};
//    CT_Grid3D<T>* grid;

//    if(reader.setFilePath(filename))
//    {
//        reader.init();
//        if(reader.readFile())
//        {
//            grid = (CT_Grid3D<T>*) reader.takeFirstItemDrawableOfType(CT_Grid3D<T>::staticGetType(), result, model);
//            grid->computeMinMax();
//        }
//    }
//    return grid;
//}

void LVOX_StepExportComputedGrids::compute()
{
    // on récupère le modèle d'entrée à exporter
    CT_InAbstractModel* in_mod{};

    // on récupère les résultats d'entrée classés par modèle d'entrée
    QList<CT_ResultGroup*> in_ress = getInputResultsForModel(DEF_in_res);
    QListIterator<CT_ResultGroup*> it(in_ress);

    QList<CT_AbstractItemDrawable*> list;

    while(it.hasNext())
    {
        CT_ResultGroup* in_res = it.next();
        in_mod = (CT_InAbstractModel*)PS_MODELS->searchModel(DEF_hits, in_res, this);

        // on recherche tous les ItemDrawable à exporter
        CT_ResultIterator it(in_res, in_mod);

        while(it.hasNext())
            list.append((CT_AbstractItemDrawable*)it.next());
    }

    // une fois la liste constituée
    if(!list.isEmpty())
    {
        PB_Grid3DExporter exporter = PB_Grid3DExporter();
//        exporter.init();
//        exporter.setExportFilePath(_folder.first() + "ni.GRD3D");

        // on la donne à l'exportateur
//        if(!exporter.setItemDrawableToExport(list))
        {
//            PS_LOG->addErrorMessage(this, exporter.errorMessage());
//            setErrorMessage(1, exporter.errorMessage());
            setErrorCode(1);
        }
//        else
        {
            //if(_exporterConfiguration != NULL)
                //_exporter->loadExportConfiguration(_exporterConfiguration);

            // et on exporte....
//            exporter.exportToFile();
        }
    }

//    CT_ResultGroup* in_res = getInputResults().first();
//    QList<CT_AbstractItemDrawable*> list = QList<CT_AbstractItemDrawable*>{};

//    CT_ResultGroupIterator it(in_res, this, DEF_in_grp);
//    while (it.hasNext() && !isStopped())
//    {
//        CT_AbstractItemGroup* in_group = (CT_AbstractItemGroup*) it.next();

////        list.push_back((const CT_Grid3D<int>*) in_group->firstItemByINModelName(this, DEF_hits));
////        list.push_back((const CT_Grid3D<int>*) in_group->firstItemByINModelName(this, DEF_theo));
////        list.push_back((const CT_Grid3D<int>*) in_group->firstItemByINModelName(this, DEF_before));
////        list.push_back((const CT_Grid3D<float>*) in_group->firstItemByINModelName(this, DEF_density));

////        PB_Grid3DExporter exporter();
////        exporter.init();
////        exporter->setExportFilePath(_folder);
//    }
}
