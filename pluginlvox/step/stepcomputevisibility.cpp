#include "stepcomputevisibility.h"

#include "ct_itemdrawable/model/outModel/ct_outstandardgroupmodel.h"
#include "ct_itemdrawable/model/outModel/ct_outstandarditemdrawablemodel.h"

#include "ct_result/model/inModel/ct_inresultmodelnotneedinputresult.h"
#include "ct_result/model/outModel/ct_outresultmodelgroup.h"

#include "ct_result/ct_resultgroup.h"
#include "ct_itemdrawable/ct_scene.h"

#include "ct_view/ct_stepconfigurabledialog.h"							// Parameter window

#include "ct_itemdrawable/ct_regulargridint.h"
#include "ct_itemdrawable/ct_scanner.h"

#include "mymaths.h"

#include <QDebug>

StepComputeVisibility::StepComputeVisibility(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
//********************************************//
//           Attributes of the grid			  //
//********************************************//
	_botX = -10;
	_botY = -10;
	_botZ = -10;
	_topX = 10;
	_topY = 10;
	_topZ = 10;
	_res = 1;

//********************************************//
//           Attributes of the scanner		  //
//********************************************//
	_scanPosX = 0;
	_scanPosY = 0;
	_scanPosZ = 0;
	_scanHFov = 40;
	_scanVFov = 40;
	_scanInitTheta = 0;
	_scanInitPhi = 90;
	_scanHRes = 0.2;
	_scanVRes = 0.2;
	_scanClockWise = false;
}

QString StepComputeVisibility::getStepDescription() const
{
    // Gives the descrption to print in the GUI
    return tr("Donne les faces visibles par le scanner en entree de chaque voxel d'une grille");
}

CT_VirtualAbstractStep* StepComputeVisibility::createNewInstance(CT_StepInitializeData &dataInit)
{
    // Creates an instance of this step
    return new StepComputeVisibility(dataInit);
}

void StepComputeVisibility::createInResultModelListProtected()
{
    CT_InResultModelNotNeedInputResult *inRes = new CT_InResultModelNotNeedInputResult();
    addInResultModel(inRes);
}

void StepComputeVisibility::createOutResultModelListProtected()
{
    addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultGroup(), QList<CT_AbstractItemDrawable*>() << new CT_RegularGridInt(), "CT_ResultRegularGridHits"));
    addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultGroup(), QList<CT_AbstractItemDrawable*>() << new CT_Scanner(), "CT_ResultScanner"));
    addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultGroup(), QList<CT_AbstractItemDrawable*>() << new CT_RegularGridDouble(), "CT_ResultSolidAngles"));
    addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultGroup(), QList<CT_AbstractItemDrawable*>() << new CT_RegularGridDouble(), "CT_ResultTheoriticalGrid"));
    addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultGroup(), QList<CT_AbstractItemDrawable*>() << new CT_RegularGridInt(), "CT_ResultVisible"));
}

void StepComputeVisibility::createConfigurationDialog()
{
    _configDialog = new CT_StepConfigurableDialog();
    _configDialog->setStep(*this);

//********************************************//
//           Attributes of the grid			  //
//********************************************//
	_configDialog->addDouble(tr("Bottom x"),tr("metres"),-10000,10000,2, _botX );
	_configDialog->addDouble(tr("Bottom y"),tr("metres"),-10000,10000,2, _botY );
	_configDialog->addDouble(tr("Bottom z"),tr("metres"),-10000,10000,2, _botZ );
	_configDialog->addDouble(tr("Top x"),tr("metres"),-10000,10000,2, _topX );
	_configDialog->addDouble(tr("Top y"),tr("metres"),-10000,10000,2, _topY );
	_configDialog->addDouble(tr("Top z"),tr("metres"),-10000,10000,2, _topZ );
	_configDialog->addDouble(tr("Resolution"),tr("metres"),-10000,10000,2, _res );

//********************************************//
//           Attributes of the scanner        //
//********************************************//
    _configDialog->addDouble(tr("Position du scanner x"),tr("metres"),-10000,10000,2, _scanPosX );
    _configDialog->addDouble(tr("Position du scanner y"),tr("metres"),-10000,10000,2, _scanPosY );
    _configDialog->addDouble(tr("Position du scanner z"),tr("metres"),-10000,10000,2, _scanPosZ );

    _configDialog->addDouble(tr("Champ de vue horizontal du scanner"),tr("degres"),0.0001,360,2, _scanHFov );
    _configDialog->addDouble(tr("Champ de vue vertical du scanner"),tr("degres"),0.0001,360,2, _scanVFov );

    _configDialog->addDouble(tr("Theta initial du scanner"),tr("degres"),-359.99,359.99,2, _scanInitTheta );
    _configDialog->addDouble(tr("Phi initial du scanner"),tr("degres"),0,180,2, _scanInitPhi );

    _configDialog->addDouble(tr("Resolution angulaire horizontale du scanner"),tr("degres"),0.0001,10000,3, _scanHRes );
    _configDialog->addDouble(tr("Resolution angulaire verticale du scanner"),tr("degres"),0.0001,10000,3, _scanVRes );

    _configDialog->addBool("Scanner sens horaire","","",_scanClockWise);
}

void StepComputeVisibility::compute()
{
    CT_ResultGroup* outResultScanner = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(1));
	
    // Creating a new scanner from the input parameters
    CT_Scanner* scanner = new CT_Scanner(0,
                                         outResultScanner,
                                         0,
                                         QVector3D(_scanPosX, _scanPosY, _scanPosZ),
                                         QVector3D(0,0,1),
                                         _scanHFov,
                                         _scanVFov,
                                         _scanHRes,
                                         _scanVRes,
                                         _scanInitTheta,
                                         _scanInitPhi,
                                         _scanClockWise );
	
    // Attaching the scanner to the corresponding result
    outResultScanner->setItemDrawable( scanner );
	
    CT_ResultGroup* outResultGrid = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(0));
    CT_ResultGroup* outResultGridSolidAngles = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(2));
    CT_ResultGroup* outResultTheoriticalGrid = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(3));
    CT_ResultGroup* outResultVisible = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(4));
	
	CT_RegularGridInt* resultGrid = new CT_RegularGridInt(0, outResultGrid, createCtPoint(_topX,_topY,_topZ,0), createCtPoint(_botX,_botY,_botZ,0), _res, -1 );
	computeVisibility( scanner->getPosition(), resultGrid );
	
    CT_RegularGridDouble* resultSolidAnglesGrid = NULL;
	computeSolidAngles( *scanner, scanner->getPosition(), resultGrid, resultSolidAnglesGrid );
	resultSolidAnglesGrid->changeResult( outResultGridSolidAngles );
	
	CT_RegularGridBool* resultVisibleGrid = new CT_RegularGridBool (0, outResultVisible, resultGrid->getTop(), resultGrid->getBot(), resultGrid->getRes() );
	flagVisible( *scanner, resultVisibleGrid );
	
    CT_RegularGridDouble* resultTheoriticalGrid = NULL;
	computeTheoriticalGridFromSolidAngles( resultSolidAnglesGrid, resultVisibleGrid, resultTheoriticalGrid, *scanner );
	resultTheoriticalGrid->changeResult( outResultTheoriticalGrid );
	
	resultGrid->calculateMinMax();
	resultSolidAnglesGrid->calculateMinMax();
	resultTheoriticalGrid->calculateMinMax();
	resultVisibleGrid->calculateMinMax();
	
	outResultGrid->setItemDrawable( resultGrid );
	outResultGridSolidAngles->setItemDrawable( resultSolidAnglesGrid );
	outResultTheoriticalGrid->setItemDrawable( resultTheoriticalGrid );
	outResultVisible->setItemDrawable( resultVisibleGrid );

    resultTheoriticalGrid->saveAsText("ResultatAvecAnglesSolide.txt");
}

void StepComputeVisibility::computeVisibility(const QVector3D& position, CT_RegularGridInt* grid)
{
	bool isInX = ( position.x() >= grid->getBot().x && position.x() <= grid->getTop().x );
	bool isInY = ( position.y() >= grid->getBot().y && position.y() <= grid->getTop().y );
	bool isInZ = ( position.z() >= grid->getBot().z && position.z() <= grid->getTop().z );
	
    int borneX = floor( (position.x() - grid->getBot().x) / grid->getRes() );
    int borneY = floor( (position.y() - grid->getBot().y) / grid->getRes() );
    int borneZ = floor( (position.z() - grid->getBot().z) / grid->getRes() );
	
	if ( borneX < 0 )
		borneX = 0;
	if ( borneY < 0 )
		borneY = 0;
	if ( borneZ < 0 )
		borneZ = 0;
	
	if ( borneX > grid->getDim().x )
		borneX = grid->getDim().x;
	if ( borneY > grid->getDim().y )
		borneY = grid->getDim().y;
	if ( borneZ > grid->getDim().z )
		borneZ = grid->getDim().z;
	
	if ( !isInX && borneX == 0 )
		borneX = -1;
	
	if ( !isInY && borneY == 0 )
		borneY = -1;
	
	if ( !isInZ && borneZ == 0 )
		borneZ = -1;
	
	CT_Point currentVoxel;
	
	if ( isInY && isInZ )
	{
		currentVoxel.y = borneY;
		currentVoxel.z = borneZ;
		for ( int i = 0 ; i < borneX ; i++ )
		{
			currentVoxel.x = i;
			grid->setData( currentVoxel, FACE1 );
		}
		
		currentVoxel.y = borneY;
		currentVoxel.z = borneZ;
		for ( int i = borneX+1 ; i < grid->getDim().x ; i++ )
		{
			currentVoxel.x = i;
			grid->setData( currentVoxel, FACE6 );
		}
	}
	
	if ( isInX && isInZ )
	{
		currentVoxel.x = borneX;
		currentVoxel.z = borneZ;
		for ( int i = 0 ; i < borneY ; i++ )
		{
			currentVoxel.y = i;
			grid->setData( currentVoxel, FACE3 );
		}
		
		currentVoxel.x = borneX;
		currentVoxel.z = borneZ;
		for ( int i = borneY+1 ; i < grid->getDim().y ; i++ )
		{
			currentVoxel.y = i;
			grid->setData( currentVoxel, FACE4 );
		}
	}
	
	if ( isInX && isInY )
	{
		currentVoxel.x = borneX;
		currentVoxel.y = borneY;
		for ( int i = 0 ; i < borneZ ; i++ )
		{
			currentVoxel.z = i;
			grid->setData( currentVoxel, FACE2 );
		}
		
		currentVoxel.x = borneX;
		currentVoxel.y = borneY;
		for ( int i = borneZ+1 ; i < grid->getDim().z ; i++ )
		{
			currentVoxel.z = i;
			grid->setData( currentVoxel, FACE5);
		}
	}
	
	if ( isInZ )
	{
		currentVoxel.z = borneZ;
		for ( int i = 0 ; i < borneX ; i++ )
		{
			for ( int j = 0 ; j < borneY ; j++ )
			{
				currentVoxel.x = i;
				currentVoxel.y = j;
				grid->setData( currentVoxel, FACE1 | FACE3);
			}
		}
		
		currentVoxel.z = borneZ;
		for ( int i = 0 ; i < borneX ; i++ )
		{
			for ( int j = borneY+1 ; j < grid->getDim().y ; j++ )
			{
				currentVoxel.x = i;
				currentVoxel.y = j;
				grid->setData( currentVoxel, FACE1 | FACE4);
			}
		}
		
		currentVoxel.z = borneZ;
		for ( int i = borneX+1 ; i < grid->getDim().x ; i++ )
		{
			for ( int j = borneY+1 ; j < grid->getDim().y ; j++ )
			{
				currentVoxel.x = i;
				currentVoxel.y = j;
				grid->setData( currentVoxel, FACE6 | FACE4);
			}
		}
		
		currentVoxel.z = borneZ;
		for ( int i = borneX+1 ; i < grid->getDim().x ; i++ )
		{
			for ( int j = 0 ; j < borneY ; j++ )
			{
				currentVoxel.x = i;
				currentVoxel.y = j;
				grid->setData( currentVoxel, FACE6 | FACE3);
			}
		}
	}
	
	if ( isInY )
	{
		currentVoxel.y = borneY;
		for ( int i = 0 ; i < borneX ; i++ )
		{
			for ( int j = 0 ; j < borneZ ; j++ )
			{
				currentVoxel.x = i;
				currentVoxel.z = j;
				grid->setData( currentVoxel, FACE1 | FACE2);
			}
		}
		
		currentVoxel.y = borneY;
		for ( int i = 0 ; i < borneX ; i++ )
		{
			for ( int j = borneZ+1 ; j < grid->getDim().z ; j++ )
			{
				currentVoxel.x = i;
				currentVoxel.z = j;
				grid->setData( currentVoxel, FACE1 | FACE5);
			}
		}
		
		currentVoxel.y = borneY;
		for ( int i = borneX+1 ; i < grid->getDim().x ; i++ )
		{
			for ( int j = borneZ+1 ; j < grid->getDim().z ; j++ )
			{
				currentVoxel.x = i;
				currentVoxel.z = j;
				grid->setData( currentVoxel, FACE6 | FACE5);
			}
		}
		
		currentVoxel.y = borneY;
		for ( int i = borneX+1 ; i < grid->getDim().x ; i++ )
		{
			for ( int j = 0 ; j < borneZ ; j++ )
			{
				currentVoxel.x = i;
				currentVoxel.z = j;
				grid->setData( currentVoxel, FACE6 | FACE2);
			}
		}
	}
	
	if ( isInX )
	{
		currentVoxel.x = borneX;
		for ( int i = 0 ; i < borneY ; i++ )
		{
			for ( int j = 0 ; j < borneZ ; j++ )
			{
				currentVoxel.y = i;
				currentVoxel.z = j;
				grid->setData( currentVoxel, FACE3 | FACE2);
			}
		}
		
		currentVoxel.x = borneX;
		for ( int i = 0 ; i < borneY ; i++ )
		{
			for ( int j = borneZ+1 ; j < grid->getDim().z ; j++ )
			{
				currentVoxel.y = i;
				currentVoxel.z = j;
				grid->setData( currentVoxel, FACE3 | FACE5);
			}
		}
		
		currentVoxel.x = borneX;
		for ( int i = borneY+1 ; i < grid->getDim().y ; i++ )
		{
			for ( int j = borneZ+1 ; j < grid->getDim().z ; j++ )
			{
				currentVoxel.y = i;
				currentVoxel.z = j;
				grid->setData( currentVoxel, FACE4 | FACE5);
			}
		}
		
		currentVoxel.x = borneX;
		for ( int i = borneY+1 ; i < grid->getDim().y ; i++ )
		{
			for ( int j = 0 ; j < borneZ ; j++ )
			{
				currentVoxel.y = i;
				currentVoxel.z = j;
				grid->setData( currentVoxel, FACE4 | FACE2);
			}
		}
	}
	
	for ( int i = 0 ; i < borneX ; i++ )
	{
		for ( int j = 0 ; j < borneY ; j++ )
		{
			for ( int k = 0 ; k < borneZ ; k++ )
			{
				currentVoxel.x = i;
				currentVoxel.y = j;
				currentVoxel.z = k;
				grid->setData( currentVoxel, FACE2 | FACE3 | FACE1);
			}
		}
	}
	
	for ( int i = 0 ; i < borneX ; i++ )
	{
		for ( int j = 0 ; j < borneY ; j++ )
		{
			for ( int k = borneZ+1 ; k < grid->getDim().z ; k++ )
			{
				currentVoxel.x = i;
				currentVoxel.y = j;
				currentVoxel.z = k;
				grid->setData( currentVoxel, FACE3 | FACE5 | FACE1);
			}
		}
	}
	
	for ( int i = 0 ; i < borneX ; i++ )
	{
		for ( int j = borneY+1 ; j < grid->getDim().y ; j++ )
		{
			for ( int k = 0 ; k < borneZ ; k++ )
			{
				currentVoxel.x = i;
				currentVoxel.y = j;
				currentVoxel.z = k;
				grid->setData( currentVoxel, FACE2 | FACE4 | FACE1);
			}
		}
	}
	
	for ( int i = 0 ; i < borneX ; i++ )
	{
		for ( int j = borneY+1 ; j < grid->getDim().y ; j++ )
		{
			for ( int k = borneZ+1 ; k < grid->getDim().z ; k++ )
			{
				currentVoxel.x = i;
				currentVoxel.y = j;
				currentVoxel.z = k;
				grid->setData( currentVoxel, FACE4 | FACE5 | FACE1);
			}
		}
	}
	
	for ( int i = borneX+1 ; i < grid->getDim().x ; i++ )
	{
		for ( int j = 0 ; j < borneY ; j++ )
		{
			for ( int k = 0 ; k < borneZ ; k++ )
			{
				currentVoxel.x = i;
				currentVoxel.y = j;
				currentVoxel.z = k;
				grid->setData( currentVoxel, FACE6 | FACE2 | FACE3);
			}
		}
	}
	
	for ( int i = borneX+1 ; i < grid->getDim().x ; i++ )
	{
		for ( int j = 0 ; j < borneY ; j++ )
		{
			for ( int k = borneZ+1 ; k < grid->getDim().z ; k++ )
			{
				currentVoxel.x = i;
				currentVoxel.y = j;
				currentVoxel.z = k;
				grid->setData( currentVoxel, FACE6 | FACE3 | FACE5);
			}
		}
	}
	
	for ( int i = borneX+1 ; i < grid->getDim().x ; i++ )
	{
		for ( int j = borneY+1 ; j < grid->getDim().y ; j++ )
		{
			for ( int k = 0 ; k < borneZ ; k++ )
			{
				currentVoxel.x = i;
				currentVoxel.y = j;
				currentVoxel.z = k;
				grid->setData( currentVoxel, FACE6 | FACE2 | FACE4);
			}
		}
	}
	
	for ( int i = borneX+1 ; i < grid->getDim().x ; i++ )
	{
		for ( int j = borneY+1 ; j < grid->getDim().y ; j++ )
		{
			for ( int k = borneZ+1 ; k < grid->getDim().z ; k++ )
			{
				currentVoxel.x = i;
				currentVoxel.y = j;
				currentVoxel.z = k;
				grid->setData( currentVoxel, FACE6 | FACE4 | FACE5);
			}
		}
	}
}

void StepComputeVisibility::computeSolidAngles(const CT_Scanner& scan, const QVector3D& position, CT_RegularGridInt* facesGrid, CT_RegularGridDouble*& outputSolidAngles)
{
	assert ( outputSolidAngles == NULL );
	
	CT_Point ctPosition = createCtPoint(  position.x(), position.y(), position.z() );
	
	// Creating the output grid
    outputSolidAngles = new CT_RegularGridDouble( 0, NULL, facesGrid->getTop(), facesGrid->getBot(), facesGrid->getRes(), 0 );
	
	// Computing for each voxel its solid angle
	unsigned int nbVoxels = facesGrid->nVoxels();
	CT_Point bot, top;
	for ( unsigned int i = 0 ; i < nbVoxels ; i++ )
	{
		// Get the bounding box of the current voxel
		facesGrid->getBBox( i, bot, top );
		
		// Les bot et top sont places dans le repere du scanner
		bot.x -= scan.getCenterX();
		bot.y -= scan.getCenterY();
		bot.z -= scan.getCenterZ();
		
		top.x -= scan.getCenterX();
		top.y -= scan.getCenterY();
		top.z -= scan.getCenterZ();
		
		int faces = facesGrid->getData(i);
		
		if ( faces & FACE1 )
		{
			outputSolidAngles->setData(i, outputSolidAngles->getData(i) + mymaths::faceSolidAngle( bot, top, FACE1, ctPosition ) );
		}
		
		if ( faces & FACE2 )
		{
			outputSolidAngles->setData(i, outputSolidAngles->getData(i) + mymaths::faceSolidAngle( bot, top, FACE2, ctPosition ) );
		}
		
		if ( faces & FACE3 )
		{
			outputSolidAngles->setData(i, outputSolidAngles->getData(i) + mymaths::faceSolidAngle( bot, top, FACE3, ctPosition ) );
		}
		
		if ( faces & FACE4 )
		{
			outputSolidAngles->setData(i, outputSolidAngles->getData(i) + mymaths::faceSolidAngle( bot, top, FACE4, ctPosition ) );
		}
		
		if ( faces & FACE5 )
		{
			outputSolidAngles->setData(i, outputSolidAngles->getData(i) + mymaths::faceSolidAngle( bot, top, FACE5, ctPosition ) );
		}
		
		if ( faces & FACE6 )
		{
			outputSolidAngles->setData(i, outputSolidAngles->getData(i) + mymaths::faceSolidAngle( bot, top, FACE6, ctPosition ) );
		}
	}
}

void StepComputeVisibility::computeTheoriticalGridFromSolidAngles(const CT_RegularGridDouble *solidAnglesGrid, const CT_RegularGridBool *visibleGrid, CT_RegularGridDouble *&outputTheoriticalGrid, const CT_Scanner &scan)
{
	assert( outputTheoriticalGrid == NULL );
	
    outputTheoriticalGrid = new CT_RegularGridDouble (0, NULL, solidAnglesGrid->getTop(), solidAnglesGrid->getBot(), solidAnglesGrid->getRes() );
	
	unsigned int nbVoxels = solidAnglesGrid->nVoxels();
	for ( unsigned int i = 0 ; i < nbVoxels ; i++ )
	{
		if ( visibleGrid->getData(i) )
			outputTheoriticalGrid->setData(i, solidAnglesGrid->getData(i) / (scan.getHRes()*scan.getVRes()) );
		
		else
			outputTheoriticalGrid->setData(i, -5 );
	}
}

void StepComputeVisibility::flagVisible(const CT_Scanner &scan, CT_RegularGridBool *&outputGrid)
{
	CT_Point voxelBot, voxelTop;
	CT_Point voxelBotSpherical, voxelTopSpherical;
	
	CT_Point p1, p2, p3, p4, p5, p6, p7, p8;
	CT_Point s1, s2, s3, s4, s5, s6, s7, s8;
	
	float thetaMax, phiMax;
	thetaMax = scan.getInitTheta() + scan.getHFov();
	phiMax = scan.getInitPhi() + scan.getVFov();
	
	unsigned int nbVoxels = outputGrid->nVoxels();
	for ( unsigned int i = 0 ; i < nbVoxels ; i++ )
	{
		outputGrid->getBBox( i, voxelBot, voxelTop );
		
		// Les bot et top sont places dans le repere du scanner
		voxelBot.x -= scan.getCenterX();
		voxelBot.y -= scan.getCenterY();
		voxelBot.z -= scan.getCenterZ();
		
		voxelTop.x -= scan.getCenterX();
		voxelTop.y -= scan.getCenterY();
		voxelTop.z -= scan.getCenterZ();
		
		fromBBoxToCorners( voxelBot, voxelTop, p1, p2, p3, p4, p5, p6, p7, p8 );
		cartesianToSpherical( p1, s1 );
		cartesianToSpherical( p2, s2 );
		cartesianToSpherical( p3, s3 );
		cartesianToSpherical( p4, s4 );
		cartesianToSpherical( p5, s5 );
		cartesianToSpherical( p6, s6 );
		cartesianToSpherical( p7, s7 );
		cartesianToSpherical( p8, s8 );
		
		outputGrid->setData(i, isInFieldOfView(scan, thetaMax, phiMax, s1) ||
							   isInFieldOfView(scan, thetaMax, phiMax, s2) ||
							   isInFieldOfView(scan, thetaMax, phiMax, s3) ||
							   isInFieldOfView(scan, thetaMax, phiMax, s4) ||
							   isInFieldOfView(scan, thetaMax, phiMax, s5) ||
							   isInFieldOfView(scan, thetaMax, phiMax, s6) ||
							   isInFieldOfView(scan, thetaMax, phiMax, s7) ||
							   isInFieldOfView(scan, thetaMax, phiMax, s8) );
	}
}

void StepComputeVisibility::cartesianToSpherical(const CT_Point& cartesian, CT_Point& spherical)
{
	/* Spherical point : x = theta y =phi z = rho */
	spherical.z = sqrt( (cartesian.x*cartesian.x)+(cartesian.y*cartesian.y)+(cartesian.z*cartesian.z) );
	spherical.y = acos( cartesian.z/spherical.z );
	
	if ( cartesian.y >= 0 )
		spherical.x = acos ( cartesian.x / sqrt( (cartesian.x*cartesian.x)+(cartesian.y*cartesian.y) ) );
	else
		spherical.x = (2*M_PI) - acos ( cartesian.x / sqrt( (cartesian.x*cartesian.x)+(cartesian.y*cartesian.y) ) );
}

void StepComputeVisibility::fromBBoxToCorners(const CT_Point &voxelBot, 
											  const CT_Point &voxelTop, 
											  CT_Point &p1, 
											  CT_Point &p2, 
											  CT_Point &p3, 
											  CT_Point &p4, 
											  CT_Point &p5, 
											  CT_Point &p6, 
											  CT_Point &p7, 
											  CT_Point &p8)
{
	// p1 p2 p3 p4 forment la face du bas dans cette fonction avec p1 = bot et on tourne dans le sens horaire
	p1 = voxelBot;
	
	p2.x = voxelBot.x;
	p2.y = voxelTop.y;
	p2.z = voxelBot.z;
	
	p3.x = voxelTop.x;
	p3.y = voxelTop.y;
	p3.z = voxelBot.z;
	
	p4.x = voxelTop.x;
	p4.y = voxelBot.y;
	p4.z = voxelBot.z;
	
	// p5 p6 p7 p8 forment la face du haut avec p5 = top et on tourne dans le sens horaire
	p5 = voxelTop;
	
	p6.x = voxelTop.x;
	p6.y = voxelBot.y;
	p6.z = voxelTop.z;
	
	p7.x = voxelBot.x;
	p7.y = voxelBot.y;
	p7.z = voxelTop.z;
	
	p8.x = voxelBot.x;
	p8.y = voxelTop.y;
	p8.z = voxelTop.z;
}

bool StepComputeVisibility::isInFieldOfView(const CT_Scanner &scan, float thetaMax, float phiMax, const CT_Point &p)
{
	return (p.x >= scan.getInitTheta() && p.x <= thetaMax && p.y >= scan.getInitPhi() && p.y <= phiMax );
}
