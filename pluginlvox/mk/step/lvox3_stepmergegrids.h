#ifndef LVOX3_STEPMERGEGRIDS_H
#define LVOX3_STEPMERGEGRIDS_H

#include "ct_step/abstract/ct_abstractstep.h"
#include "ct_tools/model/ct_autorenamemodels.h"
#include "mk/tools/lvox3_mergegrids.h"

class LVOX3_StepMergeGrids : public CT_AbstractStep
{
    Q_OBJECT

public:
    LVOX3_StepMergeGrids(CT_StepInitializeData &dataInit);

    /**
     * @brief Return a short description of what do this class
     */
    QString getStepDescription() const;

    /**
     * @brief Return a new empty instance of this class
     */
    CT_VirtualAbstractStep* createNewInstance(CT_StepInitializeData &dataInit);


protected:

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
     * @brief This method defines what kind of input the step can accept
     */
    void createInResultModelListProtected();

    /**
     * @brief This method creates a window for the user to set the different parameters of the step.
     */
    void createPostConfigurationDialog();

    /**
     * @brief This method defines what kind of output the step produces
     */
    void createOutResultModelListProtected();

    /**
     * @brief This method do the job
     */
    void compute();

private slots:
    /**
     * @brief Called when the worker progress change
     */
    void workerProgressChanged(int p);

private:
    VoxelReducerOptions m_reducerOptions;

    CT_AutoRenameModels m_mergedGridHits;
    CT_AutoRenameModels m_mergedGridBefore;
    CT_AutoRenameModels m_mergedGridTheoretic;
    CT_AutoRenameModels m_mergedGridDensity;

    CT_AutoRenameModels m_mergedNiFlag;
    CT_AutoRenameModels m_mergedNbFlag;
    CT_AutoRenameModels m_mergedNtFlag;
};

#endif // LVOX3_STEPMERGEGRIDS_H
