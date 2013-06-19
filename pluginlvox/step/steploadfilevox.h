/****************************************************************************

 Copyright (C) 2012-2012 Université de Sherbrooke, Québec, CANADA
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

#ifndef STEPLOADFILEVOX_H
#define STEPLOADFILEVOX_H

/** \file steploadfilevox.h
    \author J. Ravaglia - mail to : joris.ravaglia@gmail.com
    \version 0.1
*/

#include "ct_step/abstract/ct_abstractsteploadfileinscene.h"            // Inherits from this class
#include <vector>

using namespace std;

/** \class  StepLoadFileVOX
    \brief  Computree step loading results from lvox stored in a .vox format (Lvox voxel File)
*/
class StepLoadFileVOX : public CT_AbstractStepLoadFileInScene
{
    Q_OBJECT

public:
    StepLoadFileVOX(CT_StepInitializeData &data);

    QString getStepDescription() const;

    CT_VirtualAbstractStep* createNewInstance(CT_StepInitializeData &dataInit);
	
    QList<QString> getFileExtensionAccepted() const;

    void createOutResultModelListProtected();
	
protected:

    int readHeaderFile(QFile &f);
	
    void readDataFile(QFile &f, int offset, bool little_endian = false);
	
	void readLVoxStandAloneFile(QFile &f, int offset, bool little_endian = false);
	void readLVoxPluginFile(QFile &f, int offset, bool little_endian = false);

private :
		
    void readFloat( float& outputFloat, QStringList& stringList, int nWord, bool* success, QString msgError );

private:
    QString _defaultOpenDirPath;

    // Parametres d'en tete
    CT_Point        _bot;
    CT_Point        _top;
    unsigned int    _nVoxels;
    int             _nCategories;
    float           _res;
    vector<int>     _categoriesMarks;
	
	bool			_fromPlugin; // True if the result file comes from the LVox plugin, false if it comes from the stand alone

public slots:
    bool configure();

};

#endif // STEPLOADFILEVOX_H
