#ifndef LVOX3_STEPMERGEGRIDS_H
#define LVOX3_STEPMERGEGRIDS_H

#include "ct_step/abstract/ct_abstractstep.h"
#include "ct_tools/model/ct_autorenamemodels.h"

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

};

#endif // LVOX3_STEPMERGEGRIDS_H
