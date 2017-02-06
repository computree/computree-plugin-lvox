#include "computeprofilesconfiguration.h"
#include "ui_computeprofilesconfiguration.h"

#define CHECKPOURCENT(VALUE, ERRSTR)    if(VALUE < 0) { \
                                            if(err != NULL) \
                                                *err = tr("Valeur %1 en pourcentage invalide : %2 >= 0").arg(ERRSTR).arg(VALUE); \
                                            return false; \
                                        }

ComputeProfilesConfiguration::ComputeProfilesConfiguration(QWidget *parent) :
    CT_AbstractConfigurableWidget(parent),
    ui(new Ui::ComputeProfilesConfiguration)
{
    ui->setupUi(this);

    m_defaultConfiguration.minValue = 0;
    m_defaultConfiguration.maxValue = 999999;
    m_defaultConfiguration.ordonneeAxis = "Z";
    m_defaultConfiguration.genAxis = "X";
    m_defaultConfiguration.gridCustomized = false;
    m_defaultConfiguration.genValuesInPourcent = false;
    m_defaultConfiguration.minGen = 0;
    m_defaultConfiguration.maxGen = 999999;
    m_defaultConfiguration.stepGen = 999999;
    m_defaultConfiguration.terminalCustomized = false;
    m_defaultConfiguration.abscisseOrdonneeValuesInPourcent = false;
    m_defaultConfiguration.minAbscisse = 0;
    m_defaultConfiguration.maxAbscisse = 999999;
    m_defaultConfiguration.minOrdonnee = 0;
    m_defaultConfiguration.maxOrdonnee = 999999;

    for(int i=0; i<ui->comboBoxOrdonnee->count(); ++i)
        ui->comboBoxOrdonnee->setItemData(i, ui->comboBoxOrdonnee->itemText(i).toLower());

    on_comboBoxOrdonnee_currentIndexChanged(ui->comboBoxOrdonnee->currentIndex());
}

ComputeProfilesConfiguration::~ComputeProfilesConfiguration()
{
    delete ui;
}

void ComputeProfilesConfiguration::setConfiguration(const ComputeProfilesConfiguration::Configuration &c)
{
    ui->doubleSpinBoxMinValue->setValue(c.minValue);
    ui->doubleSpinBoxMaxValue->setValue(c.maxValue);
    ui->comboBoxOrdonnee->setCurrentIndex(ui->comboBoxOrdonnee->findData(c.ordonneeAxis.toLower()));
    on_comboBoxOrdonnee_currentIndexChanged(ui->comboBoxOrdonnee->currentIndex());
    ui->comboBoxGen->setCurrentIndex(ui->comboBoxGen->findData(c.genAxis.toLower()));

    ui->radioButtonPersonalized->setChecked(c.gridCustomized);
    ui->radioButtonAllGrid->setChecked(!c.gridCustomized);

    ui->checkBoxZValuesInPourcent->setChecked(c.genValuesInPourcent);
    ui->spinBoxMinZ->setValue(c.minGen);
    ui->spinBoxMaxZ->setValue(c.maxGen);
    ui->spinBoxGenStep->setValue(c.stepGen);

    ui->checkBoxPersonalized->setChecked(c.terminalCustomized);
    ui->checkBoxXYValuesInPourcent->setChecked(c.abscisseOrdonneeValuesInPourcent);
    ui->spinBoxMinX->setValue(c.minAbscisse);
    ui->spinBoxMaxX->setValue(c.maxAbscisse);
    ui->spinBoxMinY->setValue(c.minOrdonnee);
    ui->spinBoxMaxY->setValue(c.maxOrdonnee);
}

ComputeProfilesConfiguration::Configuration ComputeProfilesConfiguration::getConfiguration() const
{
    Configuration conf = m_defaultConfiguration;

    conf.minValue = ui->doubleSpinBoxMinValue->value();
    conf.maxValue = ui->doubleSpinBoxMaxValue->value();
    conf.ordonneeAxis = ui->comboBoxOrdonnee->currentData().toString();
    conf.genAxis = ui->comboBoxGen->currentData().toString();
    conf.gridCustomized = ui->radioButtonPersonalized->isChecked();

    if(conf.gridCustomized) {
        conf.genValuesInPourcent = ui->checkBoxZValuesInPourcent->isChecked();
        conf.minGen = ui->spinBoxMinZ->value();
        conf.maxGen = ui->spinBoxMaxZ->value();
        conf.stepGen = ui->spinBoxGenStep->value();
        conf.terminalCustomized = ui->checkBoxPersonalized->isChecked();

        if(conf.maxGen < conf.minGen)
            qSwap(conf.maxGen, conf.minGen);

        if(conf.terminalCustomized) {
            conf.abscisseOrdonneeValuesInPourcent = ui->checkBoxXYValuesInPourcent->isChecked();
            conf.minAbscisse = ui->spinBoxMinX->value();
            conf.maxAbscisse = ui->spinBoxMaxX->value();
            conf.minOrdonnee = ui->spinBoxMinY->value();
            conf.maxOrdonnee = ui->spinBoxMaxY->value();

            if(conf.maxAbscisse < conf.minAbscisse)
                qSwap(conf.maxAbscisse, conf.minAbscisse);

            if(conf.maxOrdonnee < conf.minOrdonnee)
                qSwap(conf.maxOrdonnee, conf.minOrdonnee);
        }
    }

    return conf;
}

bool ComputeProfilesConfiguration::canAcceptIt(QString *err) const
{
    if(ui->radioButtonPersonalized->isChecked()) {
        if(ui->checkBoxZValuesInPourcent->isChecked()) {
            CHECKPOURCENT(ui->spinBoxMinZ->value(), "borne z minimum");
        }

        if(ui->checkBoxPersonalized->isChecked()) {
            if(ui->checkBoxXYValuesInPourcent->isChecked()) {
                CHECKPOURCENT(ui->spinBoxMinX->value(), "borne x minimum");
                CHECKPOURCENT(ui->spinBoxMinY->value(), "borne y minimum");
            }
        }
    }

    return true;
}

bool ComputeProfilesConfiguration::isSettingsModified() const
{
    return false;
}

bool ComputeProfilesConfiguration::updateElement(QString *err)
{
    return canAcceptIt(err);
}

void ComputeProfilesConfiguration::on_pushButtonDefaultConfiguration_clicked() {
    setConfiguration(m_defaultConfiguration);
}

void ComputeProfilesConfiguration::on_comboBoxOrdonnee_currentIndexChanged(int index)
{
    ui->comboBoxGen->clear();

    if(index == 0) {
        ui->comboBoxGen->addItem("Y", "y");
        ui->comboBoxGen->addItem("Z", "z");
    } else if(index == 1) {
        ui->comboBoxGen->addItem("X", "x");
        ui->comboBoxGen->addItem("Z", "z");
    } else {
        ui->comboBoxGen->addItem("X", "x");
        ui->comboBoxGen->addItem("Y", "y");
    }
}

void ComputeProfilesConfiguration::on_radioButtonAllGrid_toggled(bool checked)
{
    ui->spinBoxMinZ->setEnabled(!checked);
    ui->spinBoxMaxZ->setEnabled(!checked);
    ui->spinBoxGenStep->setEnabled(!checked);
    ui->checkBoxPersonalized->setEnabled(!checked);
    ui->checkBoxZValuesInPourcent->setEnabled(!checked);

    if(checked)
        ui->checkBoxPersonalized->setChecked(false);
}

void ComputeProfilesConfiguration::on_checkBoxPersonalized_toggled(bool checked)
{
    ui->checkBoxXYValuesInPourcent->setEnabled(checked);
    ui->spinBoxMinX->setEnabled(checked);
    ui->spinBoxMaxX->setEnabled(checked);
    ui->spinBoxMinY->setEnabled(checked);
    ui->spinBoxMaxY->setEnabled(checked);
}
