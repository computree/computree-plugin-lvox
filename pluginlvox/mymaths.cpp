#include "mymaths.h"

#include "math.h"

#include <QDebug>

double mymaths::angleBetweenVectors(const QVector3D& v1, const QVector3D& v2)
{
    double norm1 = sqrt ( (v1.x() * v1.x()) + (v1.y()*v1.y()) + (v1.z()*v1.z()) );
    double norm2 = sqrt ( (v2.x() * v2.x()) + (v2.y()*v2.y()) + (v2.z()*v2.z()) );
    double scalarProduct = (v1.x() * v2.x() ) + ( v1.y() * v2.y() ) + ( v1.z() * v2.z() );
	
    double cos = scalarProduct / ( norm1 * norm2);

	return acos ( cos );
}

double mymaths::angleBetweenVectors(const CT_Point& v1, const CT_Point& v2)
{
	return mymaths::angleBetweenVectors( QVector3D(v1.x, v1.y, v1.z), QVector3D(v2.x, v2.y, v2.z) );
}

double mymaths::faceSolidAngle(const CT_Point& voxelBot, const CT_Point& voxelTop, unsigned int numFace, const CT_Point& pointOfView)
{
	CT_Point p1, p2, p3, p4;
	
	// Get the border point of the corresponding face
	mymaths::voxelFaceToPoints( voxelBot, voxelTop, numFace, p1, p2, p3, p4);
	
	// Change landmark so the point of view is the origin
	p1.x -= pointOfView.x; p1.y -= pointOfView.y; p1.z -= pointOfView.z;
	p2.x -= pointOfView.x; p2.y -= pointOfView.y; p2.z -= pointOfView.z;
	p3.x -= pointOfView.x; p3.y -= pointOfView.y; p3.z -= pointOfView.z;

    if ( (p1.x == 0 && p2.x == 0 && p3.x == 0) ||
         (p1.y == 0 && p2.y == 0 && p3.y == 0) ||
         (p1.z == 0 && p2.z == 0 && p3.z == 0) )
    {
        // La face est invisible par le point de vue (elle est orthogonale au plan de vue)
        return 0;
    }

	p4.x -= pointOfView.x; p4.y -= pointOfView.y; p4.z -= pointOfView.z;
	
	// Compute the six differents angles
    double alphaP1P2 = angleBetweenVectors( p1, p2 );
    double alphaP2P3 = angleBetweenVectors( p2, p3 );
    double alphaP3P4 = angleBetweenVectors( p3, p4 );
    double alphaP4P1 = angleBetweenVectors( p4, p1 );
    double alphaP1P3 = angleBetweenVectors( p1, p3 );
    double alphaP2P4 = angleBetweenVectors( p2, p4 );
	
	// Compute the cosinus of the four spherical angles of the square
    double cosP1 = ( cos(alphaP2P4) - ( cos(alphaP4P1) * cos(alphaP1P2) ) ) / ( sin(alphaP4P1) * sin(alphaP1P2) );
    double cosP2 = ( cos(alphaP1P3) - ( cos(alphaP1P2) * cos(alphaP2P3) ) ) / ( sin(alphaP1P2) * sin(alphaP2P3) );
    double cosP3 = ( cos(alphaP2P4) - ( cos(alphaP2P3) * cos(alphaP3P4) ) ) / ( sin(alphaP2P3) * sin(alphaP3P4) );
    double cosP4 = ( cos(alphaP1P3) - ( cos(alphaP3P4) * cos(alphaP4P1) ) ) / ( sin(alphaP3P4) * sin(alphaP4P1) );

    return (fabs( acos( cosP1 ) + acos( cosP2 ) + acos( cosP3 ) + acos( cosP4 ) - (2*M_PI) ));
}

void mymaths::voxelFaceToPoints(const CT_Point& voxelBot, const CT_Point& voxelTop, unsigned int numFace, CT_Point& outputP1, CT_Point& outputP2, CT_Point& outputP3, CT_Point& outputP4)
{
	switch ( numFace )
	{
		case FACE1 :
		{
			outputP1 = voxelTop;
			outputP2 = createCtPoint( voxelTop.x, voxelTop.y, voxelBot.z );
			outputP3 = createCtPoint( voxelTop.x, voxelBot.y, voxelBot.z );
			outputP4 = createCtPoint( voxelTop.x, voxelBot.y, voxelTop.z );
			break;
		}
		
		case FACE2 :
		{
			outputP1 = createCtPoint( voxelBot.x, voxelTop.y, voxelTop.z );
			outputP2 = voxelTop;
			outputP3 = createCtPoint( voxelTop.x, voxelBot.y, voxelTop.z );
			outputP4 = createCtPoint( voxelBot.x, voxelBot.y, voxelTop.z );
			break;
		}
			
		case FACE3 :
		{
			outputP1 = createCtPoint( voxelBot.x, voxelTop.y, voxelTop.z );
			outputP2 = createCtPoint( voxelBot.x, voxelTop.y, voxelBot.z );
			outputP3 = createCtPoint( voxelTop.x, voxelTop.y, voxelBot.z );
			outputP4 = voxelTop;
			break;
		}
			
		case FACE4 :
		{
			outputP1 = createCtPoint( voxelTop.x, voxelBot.y, voxelTop.z );
			outputP2 = createCtPoint( voxelTop.x, voxelBot.y, voxelBot.z );
			outputP3 = voxelBot;
			outputP4 = createCtPoint( voxelBot.x, voxelBot.y, voxelTop.z );
			break;
		}
			
		case FACE5 :
		{
			outputP1 = createCtPoint( voxelTop.x, voxelTop.y, voxelBot.z );
			outputP2 = createCtPoint( voxelBot.x, voxelTop.y, voxelBot.z );
			outputP3 = voxelBot;
			outputP4 = createCtPoint( voxelTop.x, voxelBot.y, voxelBot.z );
			break;
		}
			
		case FACE6 :
		{
			outputP1 = createCtPoint( voxelBot.x, voxelBot.y, voxelTop.z );
			outputP2 = voxelBot;
			outputP3 = createCtPoint( voxelBot.x, voxelTop.y, voxelBot.z );
			outputP4 = createCtPoint( voxelBot.x, voxelTop.y, voxelTop.z );
			break;
		}
		
		default :
		{
			qDebug() << "Mauvais numero de face, on quitte tout brutalement";
			exit(-1);
		}
	}
}
