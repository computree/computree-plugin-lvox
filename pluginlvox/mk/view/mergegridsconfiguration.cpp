#include "mergegridsconfiguration.h"
#include "ui_mergegridsconfiguration.h"
#include "mk/tools/lvox3_mergegrids.h"

MergeGridsConfiguration::MergeGridsConfiguration(QWidget *parent) :
    CT_AbstractConfigurableWidget(parent),
    ui(new Ui::MergeGridsConfiguration)
{
    ui->setupUi(this);
}

MergeGridsConfiguration::~MergeGridsConfiguration()
{
    delete ui;
}

bool MergeGridsConfiguration::isSettingsModified() const
{
    return false;
}
bool MergeGridsConfiguration::updateElement(QString *err)
{
    Q_UNUSED(err);
    return true;
}

VoxelReducerOptions MergeGridsConfiguration::getOptions()
{
    VoxelReducerOptions opts;
    opts.reducerType = (VoxelReducerType) ui->comboBoxMergeMethod->currentIndex();
    opts.ignoreVoxelZeroDensity = ui->checkBoxIgnoreZero->isChecked();
    opts.effectiveRaysThreshold = ui->spinBoxEffectiveRaysThreshold->value();
    return opts;
}

void MergeGridsConfiguration::setOptions(VoxelReducerOptions &opts)
{
    ui->checkBoxIgnoreZero->setChecked(opts.ignoreVoxelZeroDensity);
    ui->spinBoxEffectiveRaysThreshold->setValue(opts.effectiveRaysThreshold);

    ui->comboBoxMergeMethod->clear();
    for (int i = 0; i < ReducerTypeLast; i++) {
        const VoxelReducerDefinitionStruct& def = LVOX3_MergeGrids::VoxelReducerDefinitions[i];
        QString label = def.name + " " + def.desc;
        ui->comboBoxMergeMethod->addItem(label);
    }

    ui->comboBoxMergeMethod->setCurrentIndex(opts.reducerType);
    updateState();
}

void MergeGridsConfiguration::updateState()
{
    int idx = ui->comboBoxMergeMethod->currentIndex();
    ui->checkBoxIgnoreZero->setEnabled(idx == MaxTrustRatio);
    ui->spinBoxEffectiveRaysThreshold->setEnabled(idx == SumRatio);
}

void MergeGridsConfiguration::on_comboBoxMergeMethod_currentIndexChanged(int index)
{
    updateState();
}
