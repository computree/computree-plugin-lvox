#include "visitorraytracingincrementbutfirst.h"

VisitorRaytracingIncrementButFirst::VisitorRaytracingIncrementButFirst() : VisitorRaytracingIncrement()
{
	_first = true;
}

void VisitorRaytracingIncrementButFirst::visit(CT_AbstractRegularGridDataInterface *grid, int voxelID)
{
	if ( _first )
	{
		_first = false;
	}
	
	if ( !_first )
	{
		VisitorRaytracingIncrement::visit( grid, voxelID );
	}
}

void VisitorRaytracingIncrementButFirst::reset()
{
	_first = true;
}
