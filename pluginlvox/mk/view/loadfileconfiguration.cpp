#include "loadfileconfiguration.h"
#include "ui_loadfileconfiguration.h"

#include <QFileDialog>
#include <QMessageBox>

#include "ct_reader/abstract/ct_abstractreader.h"

#include "mk/tools/lvox3_scannerutils.h"

LoadFileConfiguration::LoadFileConfiguration(QWidget *parent) :
    CT_AbstractConfigurableWidget(parent),
    ui(new Ui::LoadFileConfiguration)
{
    ui->setupUi(this);

    ui->listWidgetFiles->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(ui->pushButtonModifyFile, SIGNAL(clicked(bool)), this, SLOT(modifySelectedFile()));
    connect(ui->pushButtonImportFile, SIGNAL(clicked(bool)), this, SLOT(importFile()));
    connect(ui->pushButtonDeleteFile, SIGNAL(clicked(bool)), this, SLOT(removeSelectedFile()));
    connect(ui->pushButtonDeleteAllFile, SIGNAL(clicked(bool)), this, SLOT(removeAllFile()));
    connect(ui->pushButtonAddFile, SIGNAL(clicked(bool)), this, SLOT(addFile()));

    /* load scanner types */
    /* Assumption: the scanner id matched the combobox widget order */
    for (const ScannerDefinitionStruct& scanDef: LVOX3_ScannerUtils::getScannerDefinitions()) {
        ui->comboBoxScannerType->addItem(scanDef.name);
    }

    on_listWidgetFiles_currentRowChanged(-1);
}

LoadFileConfiguration::~LoadFileConfiguration()
{
    delete ui;
}

void LoadFileConfiguration::setReaders(const QList<CT_AbstractReader *>& readers)
{
    m_availableReaders = readers;

    initReaderComboBox();
}

void LoadFileConfiguration::setCurrentReaderByClassName(const QString &readerClassName)
{
    int i = 0;

    foreach (CT_AbstractReader* reader, m_availableReaders) {
        if(reader->GetReaderClassName() == readerClassName) {
            m_currentReaderIndex = i;
            ui->comboBoxReaderType->setCurrentIndex(i);
            return;
        }

        ++i;
    }
}

void LoadFileConfiguration::setConfiguration(const QList<LoadFileConfiguration::Configuration> &configs)
{
    ui->listWidgetFiles->clear();
    m_filesScannerConfiguration.clear();

    foreach (const Configuration& c, configs) {
        QListWidgetItem* item = new QListWidgetItem(c.filepath);

        m_filesScannerConfiguration.insert(item, c);
        ui->listWidgetFiles->addItem(item);
    }

    if(!configs.isEmpty()) {
        int last = ui->listWidgetFiles->count() - 1;
        QListWidgetItem* item = ui->listWidgetFiles->item(last);
        ui->listWidgetFiles->setCurrentItem(item);
        editItem(item);
    }
}

void LoadFileConfiguration::setScannerConfigurationForced(bool enable)
{
    ui->checkBoxForceScannerInformation->setChecked(enable);
}

bool LoadFileConfiguration::isScannerConfigurationForced() const
{
    return ui->checkBoxForceScannerInformation->isChecked();
}

void LoadFileConfiguration::setFilterPointsOrigin(bool enable)
{
    ui->checkBoxFilterPointsOrigin->setChecked(enable);
}

bool LoadFileConfiguration::isFilterPointsOrigin() const
{
    return ui->checkBoxFilterPointsOrigin->isChecked();
}

QList<LoadFileConfiguration::Configuration> LoadFileConfiguration::getConfiguration() const
{
    return m_filesScannerConfiguration.values();
}

CT_AbstractReader* LoadFileConfiguration::getReaderToUse() const
{
    return m_availableReaders.at(ui->comboBoxReaderType->currentIndex());
}

bool LoadFileConfiguration::canAcceptIt(QString *err) const
{
    if(m_filesScannerConfiguration.isEmpty())
    {
        if(err != NULL)
            err->append(tr("Aucun fichier sélectionné !"));

        return false;
    }

    return true;
}

bool LoadFileConfiguration::isSettingsModified() const
{
    return false;
}

bool LoadFileConfiguration::updateElement(QString *err)
{
    if(m_filesScannerConfiguration.isEmpty())
        return false;

    QListWidgetItem* item = getSelectedItem();

    if((item != NULL) && !item->text().isEmpty())
        updateConfiguration(item);

    return true;
}

void LoadFileConfiguration::initReaderComboBox()
{
    m_currentReaderIndex = 0;
    ui->comboBoxReaderType->clear();

    foreach (CT_AbstractReader* reader, m_availableReaders) {
        ui->comboBoxReaderType->addItem(reader->GetReaderName());
    }

    ui->comboBoxReaderType->setCurrentIndex(m_currentReaderIndex);
}

void LoadFileConfiguration::editItem(QListWidgetItem *item)
{
    Configuration c = m_filesScannerConfiguration.value(item, Configuration());

    ui->comboBoxScannerType->setCurrentIndex(c.scannerId);
    ui->checkBoxScannerClockwise->setChecked(c.clockWise);
    ui->checkBoxAngleInRadians->setChecked(c.radians);

    ui->labelFilepath->setText(tr("<i>%1</i>").arg(((item == NULL) ? tr("Aucun fichier sélectionné") : QFileInfo(c.filepath).baseName())));

    ui->doubleSpinBoxXPos->setValue(c.scannerPosition.x());
    ui->doubleSpinBoxYPos->setValue(c.scannerPosition.y());
    ui->doubleSpinBoxZPos->setValue(c.scannerPosition.z());

    ui->doubleSpinBoxHRes->setValue(c.scannerResolution.x());
    ui->doubleSpinBoxVRes->setValue(c.scannerResolution.y());

    ui->doubleSpinBoxStartPhi->setValue(c.scannerPhiRange.x());
    ui->doubleSpinBoxEndPhi->setValue(c.scannerPhiRange.y());

    ui->doubleSpinBoxStartTheta->setValue(c.scannerThetaRange.x());
    ui->doubleSpinBoxEndTheta->setValue(c.scannerThetaRange.y());

    ui->doubleSpinBoxXDir->setValue(c.scannerDirection.x());
    ui->doubleSpinBoxYDir->setValue(c.scannerDirection.y());
    ui->doubleSpinBoxZDir->setValue(c.scannerDirection.z());

    setCurrentScannerType(c.scannerId);
}

void LoadFileConfiguration::updateConfiguration(QListWidgetItem *item)
{
    if(!m_filesScannerConfiguration.contains(item))
        return;

    Configuration c = m_filesScannerConfiguration.value(item);

    c.scannerId = static_cast<ScannerTypeEnum>(ui->comboBoxScannerType->currentIndex());
    c.clockWise = ui->checkBoxScannerClockwise->isChecked();
    c.radians = ui->checkBoxAngleInRadians->isChecked();

    c.scannerPosition.x() = ui->doubleSpinBoxXPos->value();
    c.scannerPosition.y() = ui->doubleSpinBoxYPos->value();
    c.scannerPosition.z() = ui->doubleSpinBoxZPos->value();

    c.scannerResolution.x() = ui->doubleSpinBoxHRes->value();
    c.scannerResolution.y() = ui->doubleSpinBoxVRes->value();

    c.scannerPhiRange.x() = ui->doubleSpinBoxStartPhi->value();
    c.scannerPhiRange.y() = ui->doubleSpinBoxEndPhi->value();

    c.scannerThetaRange.x() = ui->doubleSpinBoxStartTheta->value();
    c.scannerThetaRange.y() = ui->doubleSpinBoxEndTheta->value();

    c.scannerDirection.x() = ui->doubleSpinBoxXDir->value();
    c.scannerDirection.y() = ui->doubleSpinBoxYDir->value();
    c.scannerDirection.z() = ui->doubleSpinBoxZDir->value();

    m_filesScannerConfiguration.insert(item, c);
}

CT_AbstractReader* LoadFileConfiguration::getSelectedReader() const
{
    if((ui->comboBoxReaderType->count() == 0)
            || (ui->comboBoxReaderType->currentIndex() >= m_availableReaders.size()))
        return NULL;

    return m_availableReaders.at(ui->comboBoxReaderType->currentIndex());
}

QListWidgetItem* LoadFileConfiguration::getSelectedItem() const
{
    return ui->listWidgetFiles->currentItem();
}

void LoadFileConfiguration::addFile()
{
    QStringList list;

    if(chooseFiles(list, true)) {
        foreach (QString filepath, list) {
            QListWidgetItem* item = new QListWidgetItem(filepath);

            Configuration c;
            c.filepath = filepath;

            m_filesScannerConfiguration.insert(item, c);
            ui->listWidgetFiles->addItem(item);
        }

        int last = ui->listWidgetFiles->count()-1;
        QListWidgetItem* item = ui->listWidgetFiles->item(last);
        ui->listWidgetFiles->setCurrentItem(item);
        editItem(item);
    }
}

#include <QDebug>
#include "loadfileconfigutil.h"

void LoadFileConfiguration::importFile()
{
    // FIXME: set sensible default directory
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter(tr("Scene (*.in)"));
    dialog.setViewMode(QFileDialog::Detail);

    if (dialog.exec() == QDialog::Rejected)
        return;

    QStringList fileNames = dialog.selectedFiles();
    QList<ConfItem> conf;
    try {
        LoadFileConfigUtil::loadInFile(fileNames.at(0), conf);
        // FIXME: this should be a per-scanner config
        ui->checkBoxForceScannerInformation->setChecked(true);
    } catch (LvoxConfigError err) {
        QString msg(tr("<b>Failed to load file %1</b></br><p>%2<p>"));
        msg = msg.arg(fileNames.at(0)).arg(err.m_msg);
        QMessageBox::critical(this, tr("Import error"), msg);
        return;
    }

    QStringList notValid;
    for (const ConfItem& item: conf) {
        QFileInfo info(item.filepath);
        if (!info.exists() || !info.isFile()) {
            notValid.append(item.filepath);
        }
    }
    if (!notValid.isEmpty()) {
        QString msg(tr("<b>%n Files(s) not found:</b></br><p>%1<p>", "", notValid.count()));
        QString notValidList = notValid.join("</br>");
        msg = msg.arg(notValidList);
        QMessageBox::critical(this, tr("Import error"), msg);
        return;
    }

    setConfiguration(conf);
}

void LoadFileConfiguration::modifySelectedFile()
{
    QListWidgetItem* item = getSelectedItem();

    if(item == NULL)
        return;

    QStringList list;

    if(chooseFiles(list, false)) {
        item->setText(list.first());

        Configuration c = m_filesScannerConfiguration.value(item);
        c.filepath = list.first();

        m_filesScannerConfiguration.insert(item, c);
    }
}

bool LoadFileConfiguration::chooseFiles(QStringList& filepath, bool multiple)
{
    CT_AbstractReader* reader = getSelectedReader();

    if(reader == NULL) {
        QMessageBox::critical(this, tr("Erreur"), tr("Aucun lecteur de fichier disponible !"));
        return false;
    }

    QString formats;

    QListIterator<FileFormat> it(reader->readableFormats());

    while(it.hasNext()) {
        QListIterator<QString> itS(it.next().suffixes());

        while(itS.hasNext())
            formats += "*." + itS.next() + " ";
    }

    formats = formats.trimmed();

    if(multiple)
        filepath.append(QFileDialog::getOpenFileNames(this, tr("Choisir un fichier"), "", tr("Fichiers compatibles (%1)").arg(formats)));
    else {
        QString fp = QFileDialog::getOpenFileName(this, tr("Choisir un fichier"), "", tr("Fichiers compatibles (%1)").arg(formats));

        if(!fp.isEmpty())
            filepath.append(fp);
    }

    return !filepath.isEmpty();
}

void LoadFileConfiguration::removeSelectedFile()
{
    QListWidgetItem* item = getSelectedItem();

    if(item == NULL)
        return;

    m_filesScannerConfiguration.remove(item);

    delete item;
}

void LoadFileConfiguration::removeAllFile()
{
    m_filesScannerConfiguration.clear();
    ui->listWidgetFiles->clear();
}

void LoadFileConfiguration::on_pushButtonApplyConfigurationToNextFile_clicked()
{
    QListWidgetItem* item = getSelectedItem();

    if(item == NULL)
        return;

    updateConfiguration(item);

    int nextRow = ui->listWidgetFiles->currentRow()+1;

    if(nextRow < ui->listWidgetFiles->count()) {
        QListWidgetItem* nextItem = ui->listWidgetFiles->item(nextRow);
        const Configuration& c = m_filesScannerConfiguration.value(nextItem);
        m_filesScannerConfiguration.insert(nextItem, c);
        ui->listWidgetFiles->setCurrentRow(nextRow);
        editItem(nextItem);
    }
}

void LoadFileConfiguration::on_pushButtonApplyConfigurationToAll_clicked()
{
    QListWidgetItem* item = getSelectedItem();

    if(item == NULL)
        return;

    updateConfiguration(item);

    Configuration c = m_filesScannerConfiguration.value(item);

    QMutableHashIterator<QListWidgetItem*, Configuration> it(m_filesScannerConfiguration);

    while(it.hasNext()) {
        it.next();
        it.setValue(c);
    }
}

void LoadFileConfiguration::on_checkBoxForceScannerInformation_toggled(bool e)
{
    Q_UNUSED(e)

    on_listWidgetFiles_currentRowChanged(ui->listWidgetFiles->currentRow());
}

void LoadFileConfiguration::on_listWidgetFiles_currentRowChanged(int currentRow)
{
    bool hasRows = currentRow != -1;
    bool customChecked = ui->checkBoxForceScannerInformation->isChecked();
    bool hasRowsAndCustom = hasRows && customChecked;

    ui->pushButtonDeleteFile->setEnabled(hasRows);
    ui->pushButtonModifyFile->setEnabled(hasRows);

    ui->pushButtonApplyConfigurationToAll->setEnabled(hasRowsAndCustom);
    ui->pushButtonApplyConfigurationToNextFile->setEnabled(hasRowsAndCustom);

    if (customChecked) {
        int idx = ui->comboBoxScannerType->currentIndex();
        setCurrentScannerType(static_cast<ScannerTypeEnum>(idx));
    }
}

void LoadFileConfiguration::on_listWidgetFiles_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if((previous != NULL) && !previous->text().isEmpty())
        updateConfiguration(previous);

    editItem(current);
}

void LoadFileConfiguration::on_listWidgetFiles_itemDoubleClicked(QListWidgetItem *item)
{
    if(item->text().isEmpty())
        addFile();
    else
        modifySelectedFile();
}

void LoadFileConfiguration::on_comboBoxReaderType_currentIndexChanged(int index)
{
    ui->comboBoxReaderType->setCurrentIndex(index);
}

void LoadFileConfiguration::setCurrentScannerType(ScannerTypeEnum scannerId)
{
    bool angles = false, direction = false;

    switch(scannerId) {
    case ScannerSphericTheoretic:
        angles = true;
        break;
    case ScannerSphericPointCloud:
        break;
    case ScannerPlanePointCloud:
        direction = true;
        break;
    default:
        break;
    }

    /* scanner position is always required */
    ui->groupBoxPosition->setEnabled(true);
    ui->groupBoxAngles->setEnabled(angles);
    ui->groupBoxDirection->setEnabled(direction);
}

void LoadFileConfiguration::on_comboBoxScannerType_currentIndexChanged(int index)
{
    setCurrentScannerType(static_cast<ScannerTypeEnum>(index));
}
