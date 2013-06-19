#ifndef STEPCOMPUTEVISIBILITY_H
#define STEPCOMPUTEVISIBILITY_H

#include "ct_step/abstract/ct_abstractstep.h"                    // The step inherits from ct_abstractstep
#include "ct_point.h"
#include "ct_itemdrawable/ct_regulargridint.h"
#include "ct_itemdrawable/ct_regulargriddouble.h"
#include "ct_itemdrawable/ct_regulargridbool.h"
#include "ct_itemdrawable/ct_scanner.h"

class StepComputeVisibility : public CT_AbstractStep
{
    /** \def    Q_OBJECT :
        \brief  Macro from the Qt environment. Useful to get the name of the step (among others)*/
    Q_OBJECT

public:
    /*!
    *  \brief Constructor
    *
    *  Constructor of the class
    *  \param dataInit : informations about the step : parent step, plugin manager managing this step and some settings from Qt (for the Q_OBJECT?).
    */
    StepComputeVisibility(CT_StepInitializeData &dataInit);

    /*!
    *  \brief Gives a description of the step
    *  This method will be used in the GUI management : this description will be shown to the user when the mouse is over the step in the step menu.
    *
    *  \warning Inherited from CT_AbstractStep, this method is not pure virtual (could be not defined here).
    */
    virtual QString getStepDescription() const;

    /*!
    *  \brief Creates a new instance of this step
    *
    *  \param dataInit : informations about the step : parent step, plugin manager managing this step and some settings from Qt (for the Q_OBJECT?).
    *
    *  \warning Inherited from CT_AbstractStep, this method is pure virtual, each step MUST define its own method.
    */
    virtual CT_VirtualAbstractStep* createNewInstance(CT_StepInitializeData &dataInit);

protected:
    /*!
    *  \brief Creates the input of this step
    *
    *  This method defines what kind of input the step can accept and get them
    *
    *  \warning Inherited from CT_AbstractStep, this method is pure virtual, each step MUST define its own method.
    */
    virtual void createInResultModelListProtected();

    /*!
    *  \brief Creates the output of this step
    *
    *  This method defines what kind of results the step produces
    *
    *  \warning Inherited from CT_AbstractStep, this method is pure virtual, each step MUST define its own method.
    */
    virtual void createOutResultModelListProtected();

    /*!
    *  \brief Create a specific configuration dialog window for the step
    *
    *  This method creates a window for the user to set the different parameters of the step.
    *  The different parameters that can be set by the users and their properties (min, max, level of detail) are defined here :
    *       To add a parameter to this dialog window, just do a "_configDialog->addTYPEOFVALUE(...)"
    *
    *  \warning Inherited from CT_AbstractStep, this method is not pure virtual (could be not defined here).
    */
    virtual void createConfigurationDialog();

    /*!
    *  \brief Runs the step process
    *
    *  This method contains the effective calculus of the step (running the step is equivalent to launch this method).
    *  You might consider it as a "main" function for a step.
    *
    *  \warning Inherited from CT_AbstractStep, this method is pure virtual, each step MUST define its own method.
    */
    virtual void compute();

private :
	void computeVisibility( const QVector3D& position, CT_RegularGridInt* grid );
	
    void computeSolidAngles ( const CT_Scanner& scan, const QVector3D& position, CT_RegularGridInt* facesGrid, CT_RegularGridDouble*& outputSolidAngles );
	
    void computeTheoriticalGridFromSolidAngles ( const CT_RegularGridDouble* solidAnglesGrid, const CT_RegularGridBool* visibleGrid, CT_RegularGridDouble*& outputTheoriticalGrid, const CT_Scanner& scan );
	
	void flagVisible ( const CT_Scanner& scan, CT_RegularGridBool*& outputGrid );
	
	void cartesianToSpherical ( const CT_Point& cartesian, CT_Point& spherical );
	
	void fromBBoxToCorners ( const CT_Point& voxelBot, 
							 const CT_Point& voxelTop, 
							 CT_Point&  p1, 
							 CT_Point&  p2, 
							 CT_Point&  p3, 
							 CT_Point&  p4, 
							 CT_Point&  p5, 
							 CT_Point&  p6, 
							 CT_Point&  p7, 
							 CT_Point&  p8 );
	
	bool isInFieldOfView ( const CT_Scanner& scan, float thetaMax, float phiMax, const CT_Point& p );

private :
//********************************************//
//           Attributes of the grid			  //
//********************************************//
	double _botX;
	double _botY;
	double _botZ;
	double _topX;
	double _topY;
	double _topZ;
	double _res;

//********************************************//
//           Attributes of the scanner        //
//********************************************//
	double      _scanPosX;                  /*!< Position of the scanner in world coordinate system*/
	double      _scanPosY;                  /*!< Position of the scanner in world coordinate system*/
	double      _scanPosZ;                  /*!< Position of the scanner in world coordinate system*/
	double      _scanHFov;                  /*!< horizontal field of view*/
	double      _scanVFov;                  /*!< vertical field of view*/
	double      _scanInitTheta;             /*!< initial horizontal angle of the measurement in the world coordinate system*/
	double      _scanInitPhi;               /*!< initial vertical angle of the measurement in the world coordinate system*/
	double      _scanHRes;                  /*!< horizontal angle resolution of the scan*/
	double      _scanVRes;                  /*!< vertical angle resolution of the scan*/
	bool        _scanClockWise;             /*!< Whether the scan has been done in clockwise or not*/
};

#endif // STEPCOMPUTEVISIBILITY_H
