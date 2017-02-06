/**
 * @author Michael Krebs (AMVALOR)
 * @date 25.01.2017
 * @version 1
 */
#ifndef LVOX3_STEPCOMPUTEPROFILES_H
#define LVOX3_STEPCOMPUTEPROFILES_H

#include "ct_step/abstract/ct_abstractstep.h"
#include "ct_tools/model/ct_autorenamemodels.h"

#include "ct_itemdrawable/abstract/ct_abstractgrid3d.h"
#include "ct_itemdrawable/ct_profile.h"
#include "ct_itemdrawable/ct_standarditemgroup.h"

#include "mk/view/computeprofilesconfiguration.h"

/**
 * @brief Compute profile from grid
 */
class LVOX3_StepComputeProfiles : public CT_AbstractStep
{
    Q_OBJECT

public:
    LVOX3_StepComputeProfiles(CT_StepInitializeData &dataInit);

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
     * @brief This method defines what kind of output the step produces
     */
    void createOutResultModelListProtected();

    /**
     * @brief This method do the job
     */
    void compute();

private:
    typedef Eigen::Matrix<size_t, 3, 1> Vector3SizeT;

    CT_AutoRenameModels m_outGroupModelName;
    CT_AutoRenameModels m_outProfileModelName;

    ComputeProfilesConfiguration::Configuration m_configuration;

    /**
     * @brief Create a new profil and return it
     */
    CT_Profile<double>* createProfile(CT_ResultGroup* outResult,
                                      const CT_AbstractGrid3D* grid,
                                      const size_t& currentIndex,
                                      const Eigen::Vector3i& axeNormal,
                                      const Eigen::Vector3i& axeOrdonnee,
                                      double NAValue) const;

    /**
     * @brief Add the profile
     */
    void addProfile(CT_Profile<double>* profile, CT_ResultGroup* outResult, CT_StandardItemGroup* group);

    /**
     * @brief Modify start and end by using min, max, dim and minAndMaxInPourcent values
     */
    static void setStartEnd(size_t min, size_t max, Vector3SizeT& startEndStep, const size_t& dim, bool minAndMaxInPourcent);

    /**
     * @brief Modify step by using step and stepInPourcent values
     */
    static void setStep(size_t step, Vector3SizeT& startEndStep, bool stepInPourcent);
};

#endif // LVOX3_STEPCOMPUTEPROFILES_H
