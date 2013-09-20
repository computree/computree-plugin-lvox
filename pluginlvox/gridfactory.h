/****************************************************************************

 Copyright (C) 2012-2012 UniversitÃ© de Sherbrooke, QuÃ©bec, CANADA
                     All rights reserved.

 Contact :  richard.fournier@usherbrooke.ca
            jean-francois.cote@nrcan-rncan.gc.ca
            joris.ravaglia@gmail.com

 Developers : Joris RAVAGLIA

 This file is part of Computree version 2.0.

 Computree is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Computree is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Computree.  If not, see <http://www.gnu.org/licenses/>.

*****************************************************************************/

#ifndef GRIDFACTORY_H
#define GRIDFACTORY_H

/** \file gridfactory.h
    \author J. Ravaglia - mail to : joris.ravaglia@gmail.com
    \version 0.1
*/

#include "ct_itemdrawable/abstract/ct_abstractregulargriddatainterface.h"   // A grid factory creates some regular grid interfaces
#include "ct_itemdrawable/abstract/ct_abstractregulargriddata.h"            // A grid factory creates some regular grids
#include <QList>                                                            // A grid factory takes in parameter a list containing the grid to create

/** \class  GridFactory
    \brief  A grid factory is an object that instanciates one or several regular grid at the same time taking care about the available memory.
            If the factory can not create enough entire grids because of the lack of memory, it only allocates the same part of each grid.
            The user will have to get several parts of the grids until the entire grid has been allocate part by part.
*/
class GridFactory
{
public:    
//********************************************//
//         Constructors/Destructors           //
//********************************************//
    /*!
    *  \brief Default constructor
    *
    *  Default constructor of the class
    *  Each attribute will be set to 0 or NULL
    *  Each vector will be empty
    *
    */
    GridFactory();

    /*!
    *  \brief Constructor
    *
    *  Constructor of the class
    *
    *  \param bot : bottom left corner of the space to divide
    *  \param top : top right corner of the space to divide
    *  \param res : size of a voxel
    */
    GridFactory( const CT_Point& bot, const CT_Point& top, float resolution );

    /*!
    *  \brief Copy constructor
    *
    *  Copy constructor of the class
    *
    *  \param gridFactory : fatory to copy
    */
    GridFactory( const GridFactory& gridFactory );

    /*!
    *  \brief Destructor
    *
    *  Destructor of the class
    *  Clears the data vector
    *
    */
    ~GridFactory();

//********************************************//
//                Operators                   //
//********************************************//
    /*!
    *  \brief Assignment operator
    *
    *  Assignment operator of the class
    *
    *  \param gridFactory : fatory to copy
    */
    GridFactory& operator= (const GridFactory& gridFactory);

//********************************************//
//                  Getters                   //
//********************************************//
    /*!
    *  \brief Getter of the class
    *
    *  \return Returns the bottom left corner of the factory
    */
    inline CT_Point getBot () const { return _bot; }

    /*!
    *  \brief Getter of the class
    *
    *  \return Returns the top right corner of the factory
    */
    inline CT_Point getTop () const { return _top; }

    /*!
    *  \brief Getter of the class
    *
    *  \return Returns the number of voxel along each axis
    */
    inline CT_Point getDim () const { return _dim; }

    /*!
    *  \brief Getter of the class
    *
    *  \return Returns the size of a voxel
    */
    inline float getRes () const { return _res; }

    /*!
    *  \brief Getter of the class
    *
    *  \return Returns the index of the last voxel allocated by the factory
    */
    inline int getLastIndex () const { return _lastAllocatedIndex; }

//********************************************//
//                  PRIVATE                   //
//********************************************//
//                  Setters                   //
//********************************************//
// A priori the user should not be able to modify any of the factory settings after he created it
private :
    /*!
    *  \brief Setter of the class
    */
    inline void setBot ( const CT_Point& bot ) { _bot = bot; }

    /*!
    *  \brief Setter of the class
    */
    inline void setTop ( const CT_Point& top ) { _top = top; }

    /*!
    *  \brief Setter of the class
    */
    inline void setDim ( const CT_Point& dim ) { _dim = dim; }

    /*!
    *  \brief Setter of the class
    */
    inline void setRes ( float res ) { _res = res; }

    /*!
    *  \brief Setter of the class
    */
    inline void setLastIndex ( int lastIndex ) { _lastAllocatedIndex = lastIndex; }

//********************************************//
//      Factory methods : instanciation       //
//********************************************//
public :
    /*!
    *  \brief Instanciates the grids (entire or some parts) inserted in the QList input parameter taking care about the available memory
    *
    *  \param gridList : a list containing pointers to the grids that needs to be instanciated
    *  \return Returns true if the entire original grid has been allocated (in a single call or after several calls)
    \todo revoir la documentation de cette methode
    */
    bool nextPartsOfGrids ( QList< CT_AbstractRegularGridDataInterface* >& gridList );

//********************************************//
//                  PRIVATE                   //
//********************************************//
//                    Tools                   //
//********************************************//
private :
    /*!
    *  \brief Given a number nVoxels of voxels, returns the bounding box of the next section made of nVoxels or less voxels.
    *
    *  Also gives the next section dimensions and the number of voxel in the next section.
    *  Depending on the number of voxels given in parameter, a section can be made of :
    *  \li \c AlignLeft A set of planes
    *  \li \c AlignLeft A set of columns
    *  \li \c AlignLeft A set of voxels in the same column
    *
    *  \param nVoxels : number of voxels maximum in the next section
    *  \param outputBot : bottom left point of the section
    *  \param outputTop : top right point of the section
    *  \param outputDimensions : dimensions of the next section (number of voxels along each axis)
    */
    void getSectionBBox ( float nVoxels, float &outputNVoxels, CT_Point& outputBot, CT_Point& outputTop, CT_Point& outputDimensions );

private :
    CT_Point _bot;                              /*!< bottom left point of the entire grid to allocate*/
    CT_Point _top;                              /*!< top right point of the entire grid to allocate*/
    CT_Point _dim;                              /*!< number of voxel along each axis*/
    float   _res;                               /*!< size of the voxels in the grid*/
    int     _lastAllocatedIndex;                /*!< index of the last voxel allocated by the factory*/
};

#endif // GRIDFACTORY_H
