/**
 * @author Michael Krebs (AMVALOR)
 * @date 25.01.2017
 * @version 1
 */
#ifndef LVOX3_STEPLOADFILES_H
#define LVOX3_STEPLOADFILES_H

#include "ct_step/abstract/ct_abstractstepcanbeaddedfirst.h"
#include "ct_tools/model/ct_autorenamemodels.h"

#include "mk/view/loadfileconfiguration.h"

class LVOX3_StepLoadFiles : public CT_AbstractStepCanBeAddedFirst
{
    Q_OBJECT

public:
    LVOX3_StepLoadFiles(CT_StepInitializeData &dataInit);
    ~LVOX3_StepLoadFiles();

    /**
     * @brief Return a short description of what do this class
     */
    QString getStepDescription() const;

    /**
     * @brief Return a new empty instance of this class
     */
    CT_VirtualAbstractStep* createNewInstance(CT_StepInitializeData &dataInit);

    /**
     * @brief Backup settings (per example the file path)
     * @return The SettingsNodeGroup to save
     * @overload Overloaded from CT_AbstractStepSerializable
     */
    SettingsNodeGroup* getAllSettings() const;

    /**
     * @brief Restore settings (per example the file path)
     * @return False if it was a problem in settings
     * @overload Overloaded from CT_AbstractStepSerializable
     */
    bool setAllSettings(const SettingsNodeGroup *settings);

protected:
    /**
     * @brief This method defines what kind of input the step can accept
     */
    void createInResultModelListProtected();

    /**
     * @brief Show the post configuration dialog.
     *
     * If you want to show your own configuration dialog your must overload this method and show your dialog when this method is called. Don't forget
     * to call the method "setSettingsModified(true)" if your settings is modified (if user accept your dialog).
     *
     * @return true if the settings was modified.
     */
    bool postConfigure();

    /**
     * @brief Redefine this method if you want to init something after all configuration dialog has been called
     * @return true if your initialisation is a success
     */
    bool protectedInitAfterConfiguration();

    /**
     * @brief This method defines what kind of output the step produces
     */
    void createOutResultModelListProtected();

    /**
     * @brief This method do the job
     */
    void compute();

private:
    bool                                            m_useUserScannerConfiguration;
    bool                                            m_filterPointsOrigin;
    CT_AbstractReader*                              m_reader;
    QList<LoadFileConfiguration::Configuration>     m_configuration;

    QList<CT_AbstractReader*>                       m_availableReaders;
    CT_AutoRenameModels                             m_autoRenameFileHeader;

    /**
     * @brief Init reader list if empty
     */
    void initReaders();

    /**
     * @brief Returns a copy of the reader founded in the available list
     */
    CT_AbstractReader* getReaderByClassName(const QString& className) const;
};

#endif // LVOX3_STEPLOADFILES_H
