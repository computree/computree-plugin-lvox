#include "lvox3_steploadfiles.h"

#include <QDialog>

#include "ct_itemdrawable/model/outModel/ct_outstdgroupmodel.h"
#include "ct_itemdrawable/model/outModel/ct_outstdsingularitemmodel.h"
#include "ct_result/model/inModel/ct_inresultmodelnotneedinputresult.h"
#include "ct_result/model/outModel/ct_outresultmodelgroup.h"
#include "ct_result/ct_resultgroup.h"
#include "ct_itemdrawable/ct_standarditemgroup.h"
#include "ct_itemdrawable/ct_scanner.h"
#include "ct_model/tools/ct_modelsearchhelper.h"

#include "ct_view/tools/ct_configurablewidgettodialog.h"
#include "ct_reader/abstract/ct_abstractreader.h"
#include "ct_reader/ct_standardreaderseparator.h"
#include "ct_abstractstepplugin.h"

#define SETTING_READ(NAME, NAMEVAR, TYPE) value = groupFile->firstValueByTagName(NAME); if(value == NULL) { delete reader; return false; } NAMEVAR = value->value().TYPE();

#define DEF_outResult                "LVOX3_SLF_result"
#define DEF_outGroup                 "LVOX3_SLF_group"
#define DEF_outScannerForced         "LVOX3_SLF_scanner"

LVOX3_StepLoadFiles::LVOX3_StepLoadFiles(CT_StepInitializeData &dataInit) : CT_AbstractStepCanBeAddedFirst(dataInit)
{
    m_reader = NULL;
    m_useUserScannerConfiguration = false;
}

LVOX3_StepLoadFiles::~LVOX3_StepLoadFiles()
{
    qDeleteAll(m_availableReaders.begin(), m_availableReaders.end());

    delete m_reader;
}

QString LVOX3_StepLoadFiles::getStepDescription() const
{
    return tr("1 - Charger des fichiers pour LVOX (LVOX 3)");
}

CT_VirtualAbstractStep* LVOX3_StepLoadFiles::createNewInstance(CT_StepInitializeData &dataInit)
{
    // Creates an instance of this step
    return new LVOX3_StepLoadFiles(dataInit);
}

SettingsNodeGroup* LVOX3_StepLoadFiles::getAllSettings() const
{
    SettingsNodeGroup *root = CT_AbstractStepCanBeAddedFirst::getAllSettings();

    SettingsNodeGroup *group = new SettingsNodeGroup("LVOX3_StepLoadFiles");
    group->addValue(new SettingsNodeValue("useScannerConfiguration", m_useUserScannerConfiguration));

    SettingsNodeGroup* groupReader = new SettingsNodeGroup("Reader");

    if(m_reader != NULL) {
        groupReader->addValue(new SettingsNodeValue("name", m_reader->GetReaderClassName()));
        groupReader->addGroup(m_reader->getAllSettings());
    }

    group->addGroup(groupReader);

    int n = m_configuration.size();

    for(int i=0; i<n; ++i) {
        const LoadFileConfiguration::Configuration& c = m_configuration.at(i);

        SettingsNodeGroup *groupFile = new SettingsNodeGroup("File");
        groupFile->addValue(new SettingsNodeValue("version", 1));
        groupFile->addValue(new SettingsNodeValue("filepath", c.filepath));
        groupFile->addValue(new SettingsNodeValue("clockWise", c.clockWise));
        groupFile->addValue(new SettingsNodeValue("radians", c.radians));
        groupFile->addValue(new SettingsNodeValue("scannerPositionX", c.scannerPosition.x()));
        groupFile->addValue(new SettingsNodeValue("scannerPositionY", c.scannerPosition.y()));
        groupFile->addValue(new SettingsNodeValue("scannerPositionZ", c.scannerPosition.z()));
        groupFile->addValue(new SettingsNodeValue("scannerResolutionX", c.scannerResolution.x()));
        groupFile->addValue(new SettingsNodeValue("scannerResolutionY", c.scannerResolution.y()));
        groupFile->addValue(new SettingsNodeValue("scannerThetaStart", c.scannerThetaStartEnd.x()));
        groupFile->addValue(new SettingsNodeValue("scannerThetaEnd", c.scannerThetaStartEnd.y()));
        groupFile->addValue(new SettingsNodeValue("scannerPhiStart", c.scannerPhiStartEnd.x()));
        groupFile->addValue(new SettingsNodeValue("scannerPhiEnd", c.scannerPhiStartEnd.y()));

        group->addGroup(groupFile);
    }

    root->addGroup(group);

    return root;
}

bool LVOX3_StepLoadFiles::setAllSettings(const SettingsNodeGroup* settings)
{
    if(!CT_AbstractStepCanBeAddedFirst::setAllSettings(settings))
        return false;

    QList<SettingsNodeGroup*> listG = settings->groupsByTagName("LVOX3_StepLoadFiles");

    if(!listG.isEmpty())
    {
        QList<LoadFileConfiguration::Configuration> confs;

        bool useUserScannerConfiguration;
        CT_AbstractReader* reader = NULL;

        SettingsNodeValue* value;
        SettingsNodeGroup* groupFile = listG.first();
        SETTING_READ("useScannerConfiguration", useUserScannerConfiguration, toBool);

        QList<SettingsNodeGroup*> listConfiguration = groupFile->groupsByTagName("File");

        groupFile = groupFile->firstGroupByTagName("Reader");

        if((groupFile == NULL) || groupFile->groups().isEmpty())
            return false;

        QString readerClassName;
        SETTING_READ("name", readerClassName, toString);

        initReaders();
        reader = getReaderByClassName(readerClassName);

        if(reader == NULL)
            return false;

        reader = reader->copy();
        reader->init(false);

        if(!reader->setAllSettings(groupFile->groups().first())) {
            delete reader;
            return false;
        }

        QListIterator<SettingsNodeGroup*> it(listConfiguration);

        while(it.hasNext())
        {
            groupFile = it.next();

            LoadFileConfiguration::Configuration c;
            SETTING_READ("filepath", c.filepath, toString);
            SETTING_READ("clockWise", c.clockWise, toDouble);
            SETTING_READ("radians", c.radians, toDouble);
            SETTING_READ("scannerPositionX", c.scannerPosition.x(), toDouble);
            SETTING_READ("scannerPositionY", c.scannerPosition.y(), toDouble);
            SETTING_READ("scannerPositionZ", c.scannerPosition.z(), toDouble);
            SETTING_READ("scannerResolutionX", c.scannerResolution.x(), toDouble);
            SETTING_READ("scannerResolutionY", c.scannerResolution.y(), toDouble);
            SETTING_READ("scannerThetaStart", c.scannerThetaStartEnd.x(), toDouble);
            SETTING_READ("scannerThetaEnd", c.scannerThetaStartEnd.y(), toDouble);
            SETTING_READ("scannerPhiStart", c.scannerPhiStartEnd.x(), toDouble);
            SETTING_READ("scannerPhiEnd", c.scannerPhiStartEnd.y(), toDouble);

            confs.append(c);
        }

        delete m_reader;
        m_reader = reader;

        m_useUserScannerConfiguration = useUserScannerConfiguration;
        m_configuration = confs;

        return true;
    }

    return false;
}

void LVOX3_StepLoadFiles::createInResultModelListProtected()
{
    setNotNeedInputResult();
}

bool LVOX3_StepLoadFiles::postConfigure()
{
    initReaders();

    LoadFileConfiguration c;
    c.setReaders(m_availableReaders);
    c.setCurrentReaderByClassName(m_reader != NULL ? m_reader->GetReaderClassName() : "");
    c.setConfiguration(m_configuration);
    c.setScannerConfigurationForced(m_useUserScannerConfiguration);

    if(CT_ConfigurableWidgetToDialog::exec(&c) == QDialog::Accepted) {

        QList<LoadFileConfiguration::Configuration> configs = c.getConfiguration();
        CT_AbstractReader* reader = c.getReaderToUse()->copy();

        if(reader->setFilePath(configs.first().filepath)) {
            reader->setFilePathCanBeModified(false);
            bool ok = reader->configure();
            reader->setFilePathCanBeModified(true);

            if(ok) {
                delete m_reader;
                m_reader = reader;

                m_configuration = configs;

                m_useUserScannerConfiguration = c.isScannerConfigurationForced();

                setSettingsModified(true);

                return true;
            }
        }
    }

    return false;
}

bool LVOX3_StepLoadFiles::protectedInitAfterConfiguration()
{
    if(m_reader != NULL)
        m_reader->createOutItemDrawableModelList();

    return m_reader != NULL;
}

void LVOX3_StepLoadFiles::createOutResultModelListProtected()
{
    if(m_reader == NULL)
        return;

    CT_OutStdGroupModel *root = new CT_OutStdGroupModel(DEF_outGroup);

    QListIterator<CT_OutStdSingularItemModel*> it(m_reader->outItemDrawableModels());

    while(it.hasNext())
    {
        CT_OutStdSingularItemModel *model = it.next();
        root->addItem((CT_OutStdSingularItemModel*)model->copy());
    }

    QListIterator<CT_OutStdGroupModel*> itG(m_reader->outGroupsModel());

    while(itG.hasNext())
    {
        CT_OutStdGroupModel *model = itG.next();
        root->addGroup((CT_OutStdGroupModel*)model->copy());
    }

    CT_FileHeader *header = m_reader->createHeaderPrototype();

    if(header != NULL)
        root->addItem(new CT_OutStdSingularItemModel("", header, tr("Entête de fichier")), m_autoRenameFileHeader);

    if(m_useUserScannerConfiguration)
        root->addItem(new CT_OutStdSingularItemModel(DEF_outScannerForced, new CT_Scanner(), tr("Scanner forced"), tr("Position / Angle of the scanner forced by user")));

    addOutResultModel(new CT_OutResultModelGroup(DEF_outResult, root, "Result"));
}

void LVOX3_StepLoadFiles::compute()
{
    CT_ResultGroup *out_res = getOutResultList().first();

    int i = 0;

    QListIterator<LoadFileConfiguration::Configuration> it(m_configuration);

    while(it.hasNext())
    {
        const LoadFileConfiguration::Configuration& config = it.next();

        CT_FileHeader *header = NULL;

        if(m_reader->setFilePath(config.filepath)
                && ((header = m_reader->readHeader()) != NULL)
                && m_reader->readFile()) {

            CT_StandardItemGroup *group = new CT_StandardItemGroup(DEF_outGroup, out_res);

            QListIterator<CT_OutStdSingularItemModel*> it(m_reader->outItemDrawableModels());

            while(it.hasNext())
            {
                CT_OutStdSingularItemModel *model = it.next();
                CT_OutAbstractItemModel *modelCreation = (CT_OutAbstractItemModel*)PS_MODELS->searchModelForCreation(model->uniqueName(), out_res);

                QList<CT_AbstractSingularItemDrawable*> items = m_reader->takeItemDrawableOfModel(model->uniqueName(), out_res, modelCreation);
                QListIterator<CT_AbstractSingularItemDrawable*> itI(items);

                while(itI.hasNext())
                    group->addItemDrawable(itI.next());
            }

            QListIterator<CT_OutStdGroupModel*> itG(m_reader->outGroupsModel());

            while(itG.hasNext())
            {
                CT_OutStdGroupModel *model = itG.next();
                CT_OutAbstractItemModel *modelCreation = (CT_OutAbstractItemModel*)PS_MODELS->searchModelForCreation(model->uniqueName(), out_res);

                QList<CT_AbstractItemGroup*> groups = m_reader->takeGroupOfModel(model->uniqueName(), out_res, modelCreation);
                QListIterator<CT_AbstractItemGroup*> itI(groups);

                while(itI.hasNext())
                    group->addGroup(itI.next());
            }

            header->changeResult(out_res);
            header->setModel(m_autoRenameFileHeader.completeName());

            group->addItemDrawable(header);

            if(m_useUserScannerConfiguration) {
                // TODO : zVector !
                CT_Scanner *scanner = new CT_Scanner(DEF_outScannerForced, out_res, i,
                                                     config.scannerPosition,
                                                     Eigen::Vector3d(0,0,1),
                                                     config.scannerThetaStartEnd.y()-config.scannerThetaStartEnd.x(),
                                                     config.scannerPhiStartEnd.y()-config.scannerPhiStartEnd.x(),
                                                     config.scannerResolution.x(),
                                                     config.scannerResolution.y(),
                                                     config.scannerThetaStartEnd.x(),
                                                     config.scannerPhiStartEnd.x(),
                                                     config.clockWise,
                                                     config.radians);

                group->addItemDrawable(scanner);
            }

            out_res->addGroup(group);

            ++i;
        } else {
            delete header;
        }
    }
}

void LVOX3_StepLoadFiles::initReaders()
{
    if(m_availableReaders.isEmpty()) {
        // get the plugin manager
        PluginManagerInterface *pm = PS_CONTEXT->pluginManager();
        int s = pm->countPluginLoaded();

        // for each plugin
        for(int i=0; i<s; ++i)
        {
            CT_AbstractStepPlugin *p = pm->getPlugin(i);

            // get readers
            QList<CT_StandardReaderSeparator*> rsl = p->getReadersAvailable();
            QListIterator<CT_StandardReaderSeparator*> itR(rsl);

            while(itR.hasNext())
            {
                CT_StandardReaderSeparator *rs = itR.next();
                QListIterator<CT_AbstractReader*> itE(rs->readers());

                while(itE.hasNext())
                {
                    CT_AbstractReader *reader = itE.next()->copy();
                    reader->init(false);

                    // and add it to the list
                    m_availableReaders.append(reader);
                }
            }
        }
    }
}

CT_AbstractReader* LVOX3_StepLoadFiles::getReaderByClassName(const QString &className) const
{
    QListIterator<CT_AbstractReader*> it(m_availableReaders);

    while(it.hasNext()) {
        CT_AbstractReader* r = it.next();

        if(r->GetReaderClassName() == className) {
            return r;
        }
    }

    return NULL;
}
