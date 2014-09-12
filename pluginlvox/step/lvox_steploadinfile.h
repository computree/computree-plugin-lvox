#ifndef LVOX_STEPLOADINFILE_H
#define LVOX_STEPLOADINFILE_H

#include "ct_step/abstract/ct_abstractstepcanbeaddedfirst.h"
#include "ct_itemdrawable/model/outModel/ct_outstandarditemdrawablemodel.h"
#include "ct_itemdrawable/ct_scanner.h"

/*!
 * \class LVOX_StepLoadInFile
 * \ingroup Steps_LVOX
 * \brief <b>Load multi-scans for a plot of given radius.</b>
 *
 * This step has to be added first to load multiple points of view for the same plot
 *
 * \param _fileName Name of .in file
 * \param _radiusFiltered If true, only points inside given radius would be kept
 * \param _radius 
 *
 *
 * <b>Input Models:</b>
 *
 * No input model defined\n
 *
 * <b>Output Models:</b>
 *
 * - CT_ResultGroup (ScanPositions)\n
 *     - CT_StandardItemGroup (g)...\n
 *         - CT_Scene (ScanPoints)\n
 *         - CT_Scanner (ScanPosition)\n
 *
 * - CT_ResultGroup (MergedScan)\n
 *     - CT_StandardItemGroup (gm)...\n
 *         - CT_Scene (MergedPoints)\n
 *
 */
class LVOX_StepLoadInFile: public CT_AbstractStepCanBeAddedFirst
{
    Q_OBJECT

public:

    /*! \brief Step constructor
     * 
     * Create a new instance of the step
     * 
     * \param dataInit Step parameters object
     */
    LVOX_StepLoadInFile(CT_StepInitializeData &dataInit);

    /*! \brief Step description
     * 
     * Return a description of the step function
     */
    QString getStepDescription() const;

    /*! \brief Step copy
     * 
     * Step copy, used when a step is added by step contextual menu
     */
    CT_VirtualAbstractStep* createNewInstance(CT_StepInitializeData &dataInit);

protected:

    /*! \brief Input results specification
     * 
     * Specification of input results models needed by the step (IN)
     */
    void createInResultModelListProtected();

    /*! \brief Parameters DialogBox
     * 
     * DialogBox asking for step parameters
     */
    void createPostConfigurationDialog();

    /*! \brief Output results specification
     * 
     * Specification of output results models created by the step (OUT)
     */
    void createOutResultModelListProtected();

    /*! \brief Algorithm of the step
     * 
     * Step computation, using input results, and creating output results
     */
    void compute();

private:

    QMap<QString, CT_Scanner*>  readInFile(QString filename, Result *result);

    // Step parameters
    QStringList _fileName;          /*!< Name of .in file */
    bool        _radiusFiltered;    /*!< If true, only points inside given radius would be kept */
    double      _radius;            /*!<  */

};

#endif // LVOX_STEPLOADINFILE_H
