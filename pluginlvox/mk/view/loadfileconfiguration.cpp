#include "loadfileconfiguration.h"
#include "ui_loadfileconfiguration.h"

#include <QFileDialog>
#include <QMessageBox>

#include "ct_reader/abstract/ct_abstractreader.h"

LoadFileConfiguration::LoadFileConfiguration(QWidget *parent) :
    CT_AbstractConfigurableWidget(parent),
    ui(new Ui::LoadFileConfiguration)
{
    ui->setupUi(this);

    ui->listWidgetFiles->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(ui->pushButtonAddFile, SIGNAL(clicked(bool)), this, SLOT(addFile()));
    connect(ui->pushButtonModifyFile, SIGNAL(clicked(bool)), this, SLOT(modifySelectedFile()));
    connect(ui->pushButtonDeleteFile, SIGNAL(clicked(bool)), this, SLOT(removeSelectedFile()));
    connect(ui->pushButtonDeleteAllFile, SIGNAL(clicked(bool)), this, SLOT(removeAllFile()));

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

    if(!configs.isEmpty())
        ui->listWidgetFiles->setCurrentItem(ui->listWidgetFiles->item(ui->listWidgetFiles->count()-1));
}

void LoadFileConfiguration::setScannerConfigurationForced(bool enable)
{
    ui->checkBoxForceScannerInformation->setChecked(enable);
}

bool LoadFileConfiguration::isScannerConfigurationForced() const
{
    return ui->checkBoxForceScannerInformation->isChecked();
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

    ui->checkBoxScannerClockwise->setChecked(c.clockWise);
    ui->checkBoxAngleInRadians->setChecked(c.radians);

    ui->labelFilepath->setText(tr("<i>%1</i>").arg(((item == NULL) ? tr("Aucun fichier sélectionné") : QFileInfo(c.filepath).baseName())));

    ui->doubleSpinBoxXPos->setValue(c.scannerPosition.x());
    ui->doubleSpinBoxYPos->setValue(c.scannerPosition.y());
    ui->doubleSpinBoxZPos->setValue(c.scannerPosition.z());

    ui->doubleSpinBoxHRes->setValue(c.scannerResolution.x());
    ui->doubleSpinBoxVRes->setValue(c.scannerResolution.y());

    ui->doubleSpinBoxStartPhi->setValue(c.scannerPhiStartEnd.x());
    ui->doubleSpinBoxEndPhi->setValue(c.scannerPhiStartEnd.y());

    ui->doubleSpinBoxStartTheta->setValue(c.scannerThetaStartEnd.x());
    ui->doubleSpinBoxEndTheta->setValue(c.scannerThetaStartEnd.y());
}

void LoadFileConfiguration::updateConfiguration(QListWidgetItem *item)
{
    if(!m_filesScannerConfiguration.contains(item))
        return;

    Configuration c = m_filesScannerConfiguration.value(item);

    c.clockWise = ui->checkBoxScannerClockwise->isChecked();
    c.radians = ui->checkBoxAngleInRadians->isChecked();

    c.scannerPosition.x() = ui->doubleSpinBoxXPos->value();
    c.scannerPosition.y() = ui->doubleSpinBoxYPos->value();
    c.scannerPosition.z() = ui->doubleSpinBoxZPos->value();

    c.scannerResolution.x() = ui->doubleSpinBoxHRes->value();
    c.scannerResolution.y() = ui->doubleSpinBoxVRes->value();

    c.scannerPhiStartEnd.x() = ui->doubleSpinBoxStartPhi->value();
    c.scannerPhiStartEnd.y() = ui->doubleSpinBoxEndPhi->value();

    c.scannerThetaStartEnd.x() = ui->doubleSpinBoxStartTheta->value();
    c.scannerThetaStartEnd.y() = ui->doubleSpinBoxEndTheta->value();

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

        ui->listWidgetFiles->setCurrentItem(ui->listWidgetFiles->item(ui->listWidgetFiles->count()-1));
    }
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

    int row = ui->listWidgetFiles->currentRow()+1;

    if(row < ui->listWidgetFiles->count()) {
        m_filesScannerConfiguration.insert(ui->listWidgetFiles->item(row), m_filesScannerConfiguration.value(item));
        ui->listWidgetFiles->setCurrentRow(row);
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
    ui->pushButtonDeleteFile->setEnabled(currentRow != -1);
    ui->pushButtonModifyFile->setEnabled(currentRow != -1);

    ui->pushButtonApplyConfigurationToAll->setEnabled(currentRow != -1 && (ui->listWidgetFiles->count() > 1) && ui->checkBoxForceScannerInformation->isChecked());
    ui->pushButtonApplyConfigurationToNextFile->setEnabled(currentRow != -1 && ((currentRow+1) < ui->listWidgetFiles->count()) && ui->checkBoxForceScannerInformation->isChecked());


    ui->checkBoxAngleInRadians->setEnabled(currentRow != -1 && ui->checkBoxForceScannerInformation->isChecked());
    ui->checkBoxScannerClockwise->setEnabled(currentRow != -1 && ui->checkBoxForceScannerInformation->isChecked());
    ui->doubleSpinBoxXPos->setEnabled(currentRow != -1 && ui->checkBoxForceScannerInformation->isChecked());
    ui->doubleSpinBoxYPos->setEnabled(currentRow != -1 && ui->checkBoxForceScannerInformation->isChecked());
    ui->doubleSpinBoxZPos->setEnabled(currentRow != -1 && ui->checkBoxForceScannerInformation->isChecked());
    ui->doubleSpinBoxHRes->setEnabled(currentRow != -1 && ui->checkBoxForceScannerInformation->isChecked());
    ui->doubleSpinBoxVRes->setEnabled(currentRow != -1 && ui->checkBoxForceScannerInformation->isChecked());
    ui->doubleSpinBoxStartTheta->setEnabled(currentRow != -1 && ui->checkBoxForceScannerInformation->isChecked());
    ui->doubleSpinBoxEndTheta->setEnabled(currentRow != -1 && ui->checkBoxForceScannerInformation->isChecked());
    ui->doubleSpinBoxStartPhi->setEnabled(currentRow != -1 && ui->checkBoxForceScannerInformation->isChecked());
    ui->doubleSpinBoxEndPhi->setEnabled(currentRow != -1 && ui->checkBoxForceScannerInformation->isChecked());
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
    if((index != m_currentReaderIndex) && (ui->listWidgetFiles->count() > 0)) {
        int ret = QMessageBox::warning(this, tr("Attention"), tr("Si vous changer de lecteur de fichiers "
                                                                 "la liste des fichiers va être vider automatiquement.\r\n\r\n"
                                                                 "Voulez vous continuer ?"), QMessageBox::Yes | QMessageBox::No);

        if(ret == QMessageBox::Yes) {
            removeAllFile();
            m_currentReaderIndex = index;
        } else
            ui->comboBoxReaderType->setCurrentIndex(m_currentReaderIndex);
    }
}
