#ifndef MYMATHS_H
#define MYMATHS_H

#include <QVector3D>
#include <ct_point.h>

#define FACE1 1
#define FACE2 2
#define FACE3 4
#define FACE4 8
#define FACE5 16
#define FACE6 32

namespace mymaths
{
    /*!
    *  \brief Computes angle between two 3D vectors
    */
    double angleBetweenVectors ( const QVector3D& v1, const QVector3D& v2 );
	
    /*!
    *  \brief Computes angle between two 3D vectors (overload of the previous one)
    */
    double angleBetweenVectors ( const CT_Point& v1, const CT_Point& v2 );
	
    /*!
    *  \brief Computes the solid angle of a face of a cube from a given point of view
    */
	// Numface est le numéro de la face que l'on veut dans le voxel
    double faceSolidAngle ( const CT_Point& voxelBot, const CT_Point& voxelTop, unsigned int numFace, const CT_Point& pointOfView );
	
	void voxelFaceToPoints ( const CT_Point& voxelBot, const CT_Point& voxelTop, unsigned int numFace,
							 CT_Point& outputP1, CT_Point& outputP2, CT_Point& outputP3, CT_Point& outputP4);
}

#endif // MYMATHS_H
