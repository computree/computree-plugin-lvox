#ifndef LVOX3_STEPGENERICCOMPUTEGRIDS_H
#define LVOX3_STEPGENERICCOMPUTEGRIDS_H

#include "ct_step/abstract/ct_abstractstep.h"
#include "ct_tools/model/ct_autorenamemodels.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"

#include "ct_itemdrawable/ct_grid3d.h"

#include "mk/view/genericcomputegridsconfiguration.h"
#include "mk/tools/lvox3_genericconfiguration.h"

/**
 * @brief Compute grids with a parser to be more generic and let the user choose what operation to do
 */
class LVOX3_StepGenericComputeGrids : public CT_AbstractStep
{
    Q_OBJECT

public:
    LVOX3_StepGenericComputeGrids(CT_StepInitializeData &dataInit);

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

    /**
     * @brief Create a predefined configuration to compute the density of grids
     */
    static lvox::PredefinedConfiguration createLVOXDensityPredefinedConfiguration();

    /**
     * @brief Create a predefined configuration to compute the occlusion rate of grids
     */
    static lvox::PredefinedConfiguration createLVOXOcclusionRatePredefinedConfiguration();

protected:
    /**
     * @brief This method defines what kind of input the step can accept
     */
    void createInResultModelListProtected();

    /**
     * @brief Show the INPUT elements configuration dialog. This dialog allow the user to select elements that must be used in "compute" method
     *        for searched elements. The dialog is showed in read only mode if the step has childrens or if we can not configure something.
     * @return true if settings is modified.
     */
    bool configureInputResult(bool forceReadOnly = false);

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
     * @brief Templated method to add the desired grid to model
     */
    template<typename T>
    void addOutGridModel(int i, const QString& parentModelName, const QString& gridDisplayableName, CT_OutResultModelGroupToCopyPossibilities* res) {
        res->addItemModel(parentModelName, m_autoNameForGrid[i], new CT_Grid3D<T>(), gridDisplayableName);
    }

    /**
     * @brief This method do the job
     */
    void compute();

private:
    CT_AutoRenameModels m_autoNameForGrid[26];

    QList<lvox::InGridConfiguration>        m_input;
    QList<lvox::OutGridConfiguration>       m_output;

private slots:
    /**
     * @brief Called when the progress of the worker changed
     */
    void workerProgressChanged(int p);
};

#endif // LVOX3_STEPGENERICCOMPUTEGRIDS_H
