#include "lvox3_stepgenericcomputegrids.h"

#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"

#include "ct_result/ct_resultgroup.h"
#include "ct_itemdrawable/abstract/ct_abstractgrid3d.h"

#include "ct_turn/inTurn/tools/ct_inturnmanager.h"

#include "ct_model/inModel/tools/ct_instdmodelpossibility.h"
#include "ct_model/tools/ct_modelsearchhelper.h"

#include "ct_view/tools/ct_configurablewidgettodialog.h"

#include "mk/tools/lvox3_errorcode.h"
#include "mk/tools/lvox3_gridtype.h"
#include "mk/tools/worker/lvox3_computeall.h"
#include "mk/tools/worker/lvox3_genericcompute.h"

#include <QDialog>
#include <QMessageBox>

#define DEF_SearchInResult      "r"
#define DEF_SearchInGroup       "gr"
#define DEF_SearchInGrid        "grid"

#define READSETTINGS(STRVALUENAME, VARIABLE, CONVERTO) value = g->firstValueByTagName(STRVALUENAME); \
                                                        if(value == NULL) \
                                                            return false; \
                                                        VARIABLE = value->value().CONVERTO;

#define READCHECKSETTINGS(STRVALUENAME, VARIABLE, CONVERTO) value = checkG->firstValueByTagName(STRVALUENAME); \
                                                            if(value == NULL) \
                                                                return false; \
                                                            VARIABLE = value->value().CONVERTO;

#define ADDOUTGRIDMODEL(METATYPE, REALTYPE) if(out.gridDataType == METATYPE) { addOutGridModel<REALTYPE>(i, DEF_SearchInGroup, out.gridDisplayableName, res); }
#define CREATEOUTGRID(METATYPE, REALTYPE) if(out.gridDataType == METATYPE) { outGrid = new CT_Grid3D<REALTYPE>(m_autoNameForGrid[i].completeName(), outRes, xmin, ymin, zmin, dimx, dimy, dimz, resolution, (REALTYPE)out.gridNAValue, (REALTYPE)out.gridDefaultValue); }

LVOX3_StepGenericComputeGrids::LVOX3_StepGenericComputeGrids(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
}

QString LVOX3_StepGenericComputeGrids::getStepDescription() const
{
    return tr("3 - Calcul personnalisé sur des grilles (LVOX 3)");
}

CT_VirtualAbstractStep* LVOX3_StepGenericComputeGrids::createNewInstance(CT_StepInitializeData &dataInit)
{
    // Creates an instance of this step
    return new LVOX3_StepGenericComputeGrids(dataInit);
}

SettingsNodeGroup* LVOX3_StepGenericComputeGrids::getAllSettings() const
{
    SettingsNodeGroup *root = CT_AbstractStep::getAllSettings();

    SettingsNodeGroup *group = new SettingsNodeGroup("LVOX3_StepGenericComputeGrids");
    root->addGroup(group);

    foreach (const lvox::InGridConfiguration& in, m_input) {
        SettingsNodeGroup *inputGroup = new SettingsNodeGroup("input");
        group->addGroup(inputGroup);

        inputGroup->addValue(new SettingsNodeValue("version", 1));
        inputGroup->addValue(new SettingsNodeValue("gridLetter", QString(in.gridLetter)));
        inputGroup->addValue(new SettingsNodeValue("gridDisplayableName", in.gridDisplayableName));
        inputGroup->addValue(new SettingsNodeValue("gridType", in.gridType));
    }

    foreach (const lvox::OutGridConfiguration& out, m_output) {
        SettingsNodeGroup *outputGroup = new SettingsNodeGroup("output");
        group->addGroup(outputGroup);

        outputGroup->addValue(new SettingsNodeValue("version", 1));
        outputGroup->addValue(new SettingsNodeValue("gridLetter", QString(out.gridLetter)));
        outputGroup->addValue(new SettingsNodeValue("gridDisplayableName", out.gridDisplayableName));
        outputGroup->addValue(new SettingsNodeValue("gridDataType", (int)out.gridDataType));
        outputGroup->addValue(new SettingsNodeValue("gridNAValue", out.gridNAValue));
        outputGroup->addValue(new SettingsNodeValue("gridDefaultValue", out.gridDefaultValue));
        outputGroup->addValue(new SettingsNodeValue("formula", out.getFormulaInQStringFormat()));

        foreach (const lvox::CheckConfiguration& check, out.checks) {
            SettingsNodeGroup *checkGroup = new SettingsNodeGroup("check");
            outputGroup->addGroup(checkGroup);

            checkGroup->addValue(new SettingsNodeValue("version", 1));
            checkGroup->addValue(new SettingsNodeValue("errorFormula", check.getErrorFormulaInQStringFormat()));
            checkGroup->addValue(new SettingsNodeValue("formula", check.getFormulaInQStringFormat()));
        }
    }

    return root;
}

bool LVOX3_StepGenericComputeGrids::setAllSettings(const SettingsNodeGroup* settings)
{
    if(!CT_AbstractStep::setAllSettings(settings))
        return false;

    SettingsNodeGroup* group = settings->firstGroupByTagName("LVOX3_StepGenericComputeGrids");

    if(group == NULL)
        return false;

    SettingsNodeValue* value;
    QString formula;
    int gridDataType;
    QList<lvox::InGridConfiguration>        input;
    QList<lvox::OutGridConfiguration>       output;

    QList<SettingsNodeGroup*> groups = group->groupsByTagName("input");

    foreach (SettingsNodeGroup* g, groups) {
        lvox::InGridConfiguration in;

        READSETTINGS("gridLetter", in.gridLetter, toString().at(0).toLatin1());
        READSETTINGS("gridDisplayableName", in.gridDisplayableName, toString());
        READSETTINGS("gridType", in.gridType, toString());

        input.append(in);
    }

    groups = group->groupsByTagName("output");

    foreach (SettingsNodeGroup* g, groups) {

        QList<SettingsNodeGroup*> checkGroups = g->groupsByTagName("check");

        lvox::OutGridConfiguration out;
        lvox::CheckConfiguration check;

        foreach (SettingsNodeGroup* checkG, checkGroups) {

            READCHECKSETTINGS("errorFormula", formula, toString());

            check.setErrorFormula(formula);

            READCHECKSETTINGS("formula", formula, toString());

            check.setFormula(formula);

            out.checks.append(check);
        }

        READSETTINGS("gridLetter", out.gridLetter, toString().at(0).toLatin1());
        READSETTINGS("gridDisplayableName", out.gridDisplayableName, toString());
        READSETTINGS("gridDataType", gridDataType, toInt());
        READSETTINGS("gridNAValue", out.gridNAValue, toDouble());
        READSETTINGS("gridDefaultValue", out.gridDefaultValue, toDouble());
        READSETTINGS("formula", formula, toString());

        out.gridDataType = (QMetaType::Type)gridDataType;
        out.setFormula(formula);

        output.append(out);
    }

    m_input = input;
    m_output = output;

    return true;
}

lvox::PredefinedConfiguration LVOX3_StepGenericComputeGrids::createLVOXDensityPredefinedConfiguration()
{
    lvox::PredefinedConfiguration conf;
    conf.displayableName = QObject::tr("LVOX Densité");
    conf.description = QObject::tr("Calcul d'une grille de densité à partir des grilles \"Nt\", \"Nb\" et \"Ni\"");

    lvox::InGridConfiguration in;
    in.gridLetter = 'a';
    in.gridType = lvox::Grid3Di::staticGetType();
    in.gridDisplayableName =  QObject::tr("Hits [%1]").arg(lvox::Grid3Di::staticName());

    conf.input.append(in);

    in.gridLetter = 'b';
    in.gridDisplayableName =  QObject::tr("Theoretical [%1]").arg(lvox::Grid3Di::staticName());

    conf.input.append(in);

    in.gridLetter = 'c';
    in.gridDisplayableName =  QObject::tr("Before [%1]").arg(lvox::Grid3Di::staticName());

    conf.input.append(in);

    lvox::OutGridConfiguration out;
    out.gridLetter = 'a';
    out.gridDataType = lvox::Grid3DfMetaType();
    out.gridDisplayableName =  QObject::tr("Density");
    out.gridNAValue = lvox::Max_Error_Code;
    out.gridDefaultValue = out.gridNAValue;
    out.setFormula("a / (b - c)");

    lvox::CheckConfiguration check;
    check.setFormula("b == c");
    check.setErrorFormula(QString().setNum(lvox::B_Equals_C));

    out.checks.append(check);

    check.setFormula("b < c");
    check.setErrorFormula(QString().setNum(lvox::B_Inferior_C));

    out.checks.append(check);

    check.setFormula("(b - c) < 10");
    check.setErrorFormula(QString().setNum(lvox::B_Minus_C_Inferior_Threshold));

    out.checks.append(check);

    check.setFormula("a > (b - c)");
    check.setErrorFormula(QString().setNum(lvox::A_Superior_B_Minus_C));

    out.checks.append(check);

    conf.output.append(out);

    return conf;
}

lvox::PredefinedConfiguration LVOX3_StepGenericComputeGrids::createLVOXOcclusionRatePredefinedConfiguration()
{
    lvox::PredefinedConfiguration conf;
    conf.displayableName = QObject::tr("LVOX Taux d'occlusion");
    conf.description = QObject::tr("Calcul d'une grille contenant le taux d'occlusion à partir des grilles \"Nt\" et \"Nb\"");

    lvox::InGridConfiguration in;
    in.gridLetter = 'a';
    in.gridType = lvox::Grid3Di::staticGetType();
    in.gridDisplayableName =  QObject::tr("Theoretical [%1]").arg(lvox::Grid3Di::staticName());

    conf.input.append(in);

    in.gridLetter = 'b';
    in.gridDisplayableName =  QObject::tr("Before [%1]").arg(lvox::Grid3Di::staticName());

    conf.input.append(in);

    lvox::OutGridConfiguration out;
    out.gridLetter = 'a';
    out.gridDataType = lvox::Grid3DfMetaType();
    out.gridDisplayableName =  QObject::tr("Taux d'occlusion");
    out.gridNAValue = lvox::Max_Error_Code;
    out.gridDefaultValue = out.gridNAValue;
    out.setFormula("(a - b) / a");

    lvox::CheckConfiguration check;
    check.setFormula("a <= 0");
    check.setErrorFormula(QString().setNum(lvox::Zero_Division));

    out.checks.append(check);

    check.setFormula("a < b");
    check.setErrorFormula(QString().setNum(lvox::Nt_Inferior_Nb));

    out.checks.append(check);

    conf.output.append(out);

    return conf;
}

void LVOX3_StepGenericComputeGrids::createInResultModelListProtected()
{
    // We must have
    // - at least one grid
    // - and X grid if the user want to do some compute on multiple grid

    CT_InResultModelGroupToCopy *resultScan = createNewInResultModelForCopy(DEF_SearchInResult, tr("Grilles"), "", true);
    resultScan->setZeroOrMoreRootGroup();
    resultScan->addGroupModel("", DEF_SearchInGroup, CT_AbstractItemGroup::staticGetType());

    QString modelName = QString(DEF_SearchInGrid);

    // TODO : change it in CT_AbstractVector when it will be available in pluginshared ! (we must just know how many case it was and change the value at an index)
    resultScan->addItemModel(DEF_SearchInGroup, modelName + 'a', CT_AbstractGrid3D::staticGetType(), tr("Grille A"));

    // add multiple optionnal grid to let the user choose as many as he want
    for(char c = 'b'; c <= 'z'; ++c)
        resultScan->addItemModel(DEF_SearchInGroup, modelName + c, CT_AbstractGrid3D::staticGetType(), tr("Grille %1").arg(QString(c).toUpper()), tr("Grille optionnel à utiliser si vous voulez faire des calculs sur plus d'une grille"), CT_InAbstractModel::C_ChooseOneIfMultiple, CT_InAbstractModel::F_IsOptional);
}

bool LVOX3_StepGenericComputeGrids::configureInputResult(bool forceReadOnly)
{
    if(!getStepChildList().isEmpty() || m_output.isEmpty() || forceReadOnly)
        return CT_AbstractStep::configureInputResult(forceReadOnly);

    int ret = QMessageBox::warning(NULL, tr("Attention"), tr("Vous avez déjà définie des grilles de sorties vous ne "
                                                    "pouvez donc pas changer les grilles d'entrées. Voulez vous "
                                                    "supprimer votre configuration des grilles de sorties pour modifier "
                                                    "les grilles d'entrées ?"), QMessageBox::Yes | QMessageBox::No);

    if(ret == QMessageBox::Yes) {
        if(CT_AbstractStep::configureInputResult(false)) {
            m_output.clear();

            showPostConfigurationDialog();
            initAfterConfiguration();
            return true;
        }

        return false;
    }

    return CT_AbstractStep::configureInputResult(true);
}

bool LVOX3_StepGenericComputeGrids::postConfigure()
{
    QList<lvox::InGridConfiguration> input;
    QList<lvox::OutGridConfiguration> output;

    if(m_input.isEmpty()) {
        // Trick for use the model
        // TODO : remove it when turn will be removed in ComputreeV5
        getInTurnManager()->getTurnIndexManager()->resetTurnIndex();
        getInTurnManager()->createSearchModelListForCurrentTurn();

        CT_InAbstractResultModel *resultInModel = getInResultModel(DEF_SearchInResult);

        if((resultInModel == NULL) || !resultInModel->isAtLeastOnePossibilitySelected())
            return false;

        CT_OutAbstractResultModel* resultInModelOut = (CT_OutAbstractResultModel*)resultInModel->getPossibilitiesSavedSelected().first()->outModel();

        for(char c = 'a'; c <= 'z'; ++c) {
            QString modelName = QString(DEF_SearchInGrid);
            modelName += c;
            CT_InAbstractModel* model = dynamic_cast<CT_InAbstractModel*>(PS_MODELS->searchModel(modelName, resultInModelOut, this));

            if((model != NULL) && model->isAtLeastOnePossibilitySelected()) {
                const CT_OutAbstractModel* outModel = model->getPossibilitiesSavedSelected().first()->outModel();
                const CT_AbstractItemDrawable* itemDrawable = (CT_AbstractItemDrawable*)outModel->item();

                lvox::InGridConfiguration conf;
                conf.gridLetter = c;
                conf.gridType = itemDrawable->getType();
                conf.gridDisplayableName = outModel->displayableName() + " [" + itemDrawable->name() + "]";
                input.append(conf);
            } else if(c == 'a')  {
                return false;
            }
        }

        if(input.isEmpty())
            return false;

        lvox::OutGridConfiguration conf;
        conf.gridLetter = 'a';
        conf.gridDataType = QMetaType::Float;
        conf.gridDisplayableName = tr("Grille calculée");

        output.append(conf);

    } else {
        input = m_input;
        output = m_output;
    }

    GenericComputeGridsConfiguration config;
    config.setInputGridConfiguration(input);
    config.setOutputGridConfiguration(output);
    config.addPredefinedConfiguration(createLVOXDensityPredefinedConfiguration());
    config.addPredefinedConfiguration(createLVOXOcclusionRatePredefinedConfiguration());

    if(CT_ConfigurableWidgetToDialog::exec(&config) == QDialog::Accepted) {

        m_input = config.getInputGridConfiguration();
        m_output = config.getOutputGridConfiguration();

        setSettingsModified(true);        

        return true;
    }

    return false;
}

void LVOX3_StepGenericComputeGrids::createOutResultModelListProtected()
{
    // create a new OUT result that is a copy of the IN result selected by the user
    CT_OutResultModelGroupToCopyPossibilities *res = createNewOutResultModelToCopy(DEF_SearchInResult);

    if (res != NULL)
    {
        int n = m_output.size();

        for(int i=0; i<n; ++i) {
            const lvox::OutGridConfiguration& out = m_output.at(i);

            ADDOUTGRIDMODEL(QMetaType::Bool, bool);
            ADDOUTGRIDMODEL(QMetaType::Int, int);
            ADDOUTGRIDMODEL(QMetaType::UInt, uint);
            ADDOUTGRIDMODEL(QMetaType::LongLong, qlonglong);
            ADDOUTGRIDMODEL(QMetaType::ULongLong, qulonglong);
            ADDOUTGRIDMODEL(QMetaType::Double, double);
            ADDOUTGRIDMODEL(QMetaType::Long, long);
            ADDOUTGRIDMODEL(QMetaType::Short, short);
            ADDOUTGRIDMODEL(QMetaType::Char, char);
            ADDOUTGRIDMODEL(QMetaType::ULong, ulong);
            ADDOUTGRIDMODEL(QMetaType::UShort, ushort);
            ADDOUTGRIDMODEL(QMetaType::UChar, uchar);
            ADDOUTGRIDMODEL(QMetaType::Float, float);
        }
    }
}

void LVOX3_StepGenericComputeGrids::compute()
{
    CT_ResultGroup *outRes = getOutResultList().first();

    LVOX3_ComputeAll computeManager;

    connect(&computeManager, SIGNAL(progressChanged(int)), this, SLOT(workerProgressChanged(int)), Qt::DirectConnection);
    connect(this, SIGNAL(stopped()), &computeManager, SLOT(cancel()), Qt::DirectConnection);

    double xmin;
    double ymin;
    double zmin;
    size_t dimx;
    size_t dimy;
    size_t dimz;
    double resolution;

    CT_ResultGroupIterator itGrp(outRes, this, DEF_SearchInGroup);

    while (itGrp.hasNext())
    {
        CT_AbstractItemGroup *group = (CT_AbstractItemGroup*) itGrp.next();

        QList<LVOX3_GenericCompute::Input> workerInputs;

        foreach (const lvox::InGridConfiguration& in, m_input) {

            const QString modelName = QString(DEF_SearchInGrid) + in.gridLetter;

            LVOX3_GenericCompute::Input win;
            win.grid = (CT_AbstractGrid3D*)group->firstItemByINModelName(this, modelName);

            if(win.grid == NULL)
                return;

            win.gridLetterInFormula = in.gridLetter;

            workerInputs.append(win);
        }

        CT_AbstractGrid3D* inGrid = workerInputs.first().grid;
        xmin = inGrid->minX();
        ymin = inGrid->minY();
        zmin = inGrid->minZ();
        dimx = inGrid->xdim();
        dimy = inGrid->ydim();
        dimz = inGrid->zdim();
        resolution = inGrid->resolution();

        int i = 0;

        foreach (const lvox::OutGridConfiguration& out, m_output) {

            CT_AbstractGrid3D* outGrid = NULL;

            CREATEOUTGRID(QMetaType::Bool, bool);
            CREATEOUTGRID(QMetaType::Int, int);
            CREATEOUTGRID(QMetaType::UInt, uint);
            CREATEOUTGRID(QMetaType::LongLong, qlonglong);
            CREATEOUTGRID(QMetaType::ULongLong, qulonglong);
            CREATEOUTGRID(QMetaType::Double, double);
            CREATEOUTGRID(QMetaType::Long, long);
            CREATEOUTGRID(QMetaType::Short, short);
            CREATEOUTGRID(QMetaType::Char, char);
            CREATEOUTGRID(QMetaType::ULong, ulong);
            CREATEOUTGRID(QMetaType::UShort, ushort);
            CREATEOUTGRID(QMetaType::UChar, uchar);
            CREATEOUTGRID(QMetaType::Float, float);

            group->addItemDrawable(outGrid);

            LVOX3_GenericCompute* worker = new LVOX3_GenericCompute(workerInputs, out.checks, out.getFormula(), outGrid);
            computeManager.addWorker(0, worker);

            ++i;
        }
    }

    computeManager.compute();

}

void LVOX3_StepGenericComputeGrids::workerProgressChanged(int p)
{
    setProgress(p);
}
