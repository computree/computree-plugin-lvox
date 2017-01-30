#ifndef LOADFILECONFIGURATION_H
#define LOADFILECONFIGURATION_H

#include <QWidget>

#include "Eigen/Core"

#include "ct_view/ct_abstractconfigurablewidget.h"

namespace Ui {
class LoadFileConfiguration;
}

class QListWidgetItem;
class CT_AbstractReader;

class LoadFileConfiguration : public CT_AbstractConfigurableWidget
{
    Q_OBJECT

public:
    struct Configuration {
        Configuration() {
            clockWise = true;
            radians = false;
        }

        bool clockWise;
        bool radians;
        QString filepath;
        Eigen::Vector3d scannerPosition;
        Eigen::Vector2d scannerResolution;
        Eigen::Vector2d scannerThetaStartEnd;
        Eigen::Vector2d scannerPhiStartEnd;
    };

    explicit LoadFileConfiguration(QWidget *parent = 0);
    ~LoadFileConfiguration();

    /**
     * @brief Set readers to use
     */
    void setReaders(const QList<CT_AbstractReader*>& readers);

    /**
     * @brief Set the current reader
     */
    void setCurrentReaderByClassName(const QString& readerClassName);

    /**
     * @brief Set all configurations
     */
    void setConfiguration(const QList<Configuration>& configs);

    /**
     * @brief Set to true to foce scanner configuration
     */
    void setScannerConfigurationForced(bool enable);

    /**
     * @brief Returns true if the scanner configuration must be used in configuration object
     */
    bool isScannerConfigurationForced() const;

    /**
     * @brief Returns all configuration
     */
    QList<Configuration> getConfiguration() const;

    /**
     * @brief Return the reader to use (you must copy it to use it!)
     */
    CT_AbstractReader* getReaderToUse() const;

    /**
     * @brief Returns true if it was no error and "updateElement" method can be called
     * @param err : (optionnal) will not be empty if an error can be displayed and if different of NULL
     */
    bool canAcceptIt(QString *err = NULL) const;

    /**
     * @brief Returns true if user has modified something
     */
    bool isSettingsModified() const;

public slots:
    /**
     * @brief Update the element to match the configuration defined by the user
     * @param err : (optionnal) will not be empty if an error can be displayed and if different of NULL
     * @return true if update was successfull, otherwise returns false and filled the "err" parameter if it was not NULL
     */
    bool updateElement(QString *err = NULL);

private:

    Ui::LoadFileConfiguration               *ui;
    QHash<QListWidgetItem*, Configuration>  m_filesScannerConfiguration;
    QList<CT_AbstractReader*>               m_availableReaders;
    int                                     m_currentReaderIndex;

    void initReaderComboBox();

    void editItem(QListWidgetItem *item);
    void updateConfiguration(QListWidgetItem *item);

    bool chooseFiles(QStringList &filepath, bool multiple);

    CT_AbstractReader* getSelectedReader() const;
    QListWidgetItem* getSelectedItem() const;

private slots:
    void addFile();
    void modifySelectedFile();
    void removeSelectedFile();
    void removeAllFile();

    void on_pushButtonApplyConfigurationToNextFile_clicked();
    void on_pushButtonApplyConfigurationToAll_clicked();

    void on_checkBoxForceScannerInformation_toggled(bool e);

    void on_listWidgetFiles_currentRowChanged(int currentRow);
    void on_listWidgetFiles_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void on_listWidgetFiles_itemDoubleClicked(QListWidgetItem *item);

    void on_comboBoxReaderType_currentIndexChanged(int index);
};

#endif // LOADFILECONFIGURATION_H
