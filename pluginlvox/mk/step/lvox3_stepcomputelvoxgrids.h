/**
 * @author Michael Krebs (AMVALOR)
 * @date 25.01.2017
 * @version 1
 */
#ifndef LVOX3_STEPCOMPUTELVOXGRIDS_H
#define LVOX3_STEPCOMPUTELVOXGRIDS_H

#include "ct_step/abstract/ct_abstractstep.h"
#include "ct_tools/model/ct_autorenamemodels.h"

/**
 * @brief Compute the LVOX Ni, Nb, Nt grids
 */
class LVOX3_StepComputeLvoxGrids : public CT_AbstractStep
{
    Q_OBJECT

public:
    LVOX3_StepComputeLvoxGrids(CT_StepInitializeData &dataInit);

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

private:
    CT_AutoRenameModels _deltaout_ModelName;
    CT_AutoRenameModels _deltain_ModelName;
    CT_AutoRenameModels _deltabef_ModelName;
    CT_AutoRenameModels _deltatheo_ModelName;
    CT_AutoRenameModels _bef_ModelName;
    CT_AutoRenameModels _theo_ModelName;
    CT_AutoRenameModels _hits_ModelName;
    CT_AutoRenameModels _density_ModelName;

    CT_AutoRenameModels _NiFlag_ModelName;
    CT_AutoRenameModels _NbFlag_ModelName;
    CT_AutoRenameModels _NtFlag_ModelName;
    CT_AutoRenameModels _DensityFlag_ModelName;

//********************************************//
//              Attributes of LVox            //
//********************************************//

    double          m_resolution;               /*!< size of a voxel */
    bool            m_computeDistances;         /*!< true if must compute distance */
    int             m_gridMode;                 /*!< grid mode */
    Eigen::Vector3d m_coordinates;              /*!< coordinates if gridMode == ...Coordinates... */
    Eigen::Vector3i m_dimensions;               /*!< dimensions if gridMode == ...CustomDimensions */
    QStringList     m_gridFilePath;             /*!< Name of .grid L-Architect reference 3D grid */

private slots:
    /**
     * @brief Called from worker manager when progress changed
     */
    void progressChanged(int p);
};

#endif // LVOX3_STEPCOMPUTELVOXGRIDS_H
