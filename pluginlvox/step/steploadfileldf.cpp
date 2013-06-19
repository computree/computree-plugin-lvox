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

#include "steploadfileldf.h"

#include "ct_itemdrawable/ct_regulargridint.h"
#include "ct_itemdrawable/ct_regulargriddouble.h"

#include "ct_step/ct_stepinitializedata.h"
#include "ct_pointcloudstdvector.h"
#include "ct_pointcloudindexvector.h"
#include <limits>

#include <QFileDialog>
#include <QTextStream>
#include <QDebug>

StepLoadFileLDF::StepLoadFileLDF(CT_StepInitializeData &data) : CT_AbstractStepLoadFileInScene(data)
{
    _nCategories = 1;

    QSettings *settings = data.getSettingsFile();

    if(settings != NULL)
    {
        settings->beginGroup(getStepName());

        _defaultOpenDirPath = settings->value("defaultOpenDirPath", ".").toString();

        settings->endGroup();
    }
}


QString StepLoadFileLDF::getStepDescription() const
{
    return tr("Charge un fichier .ldf");
}

CT_VirtualAbstractStep* StepLoadFileLDF::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new StepLoadFileLDF(dataInit);
}

QList<QString> StepLoadFileLDF::getFileExtensionAccepted() const
{
    QList<QString> list;
    list << ".ldf";

    return list;
}

void StepLoadFileLDF::createOutResultModelListProtected()
{
    // Reading the header of the file to know how many categoy there is
    if(QFile::exists( getFilePath() ))
    {
        QFile f( getFilePath() );
        if(f.open(QIODevice::ReadOnly))
        {
            readHeaderFile(f);
        }
        f.close();
    }

	if ( _nCategories > 1 )
	{
		for ( int i = 0 ; i < _nCategories ; i++ )
		{
            addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultGroup(), QList<CT_AbstractItemDrawable*>() << new CT_RegularGridInt(), QString("HitGridCategory").append(QString().number(i)) ) );
		}
	}

    addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultGroup(), QList<CT_AbstractItemDrawable*>() << new CT_RegularGridInt(), "BeforeGrid"));
    addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultGroup(), QList<CT_AbstractItemDrawable*>() << new CT_RegularGridInt(), "TheoriticalGrid"));
    addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultGroup(), QList<CT_AbstractItemDrawable*>() << new CT_RegularGridInt(), "HitsGrid"));
    addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultGroup(), QList<CT_AbstractItemDrawable*>() << new CT_RegularGridDouble(), "DensityGrid"));
}

int StepLoadFileLDF::readHeaderFile(QFile &f)
{
    qint64      currentSizeRead = 0;
    QTextStream stream ( &f );
    QString     currentLine;
    QStringList wordsOfLine;

////// The first ligne should be the bounding box of the grids
    currentLine = stream.readLine();
    currentSizeRead += currentLine.size()+1; // +1 for the "\n" char
	
	// Skip comments
	if ( currentLine[0] == '#' )
	{
		while ( currentLine[0] == '#' )
		{
			currentLine = stream.readLine();
			currentSizeRead += currentLine.size()+1; // +1 for the "\n" char
		}
	}

    // Read each word separately
    wordsOfLine = currentLine.split( QRegExp("\\s+"), QString::SkipEmptyParts );

    // Checking for a valid line
    if ( wordsOfLine.size() != 6 )
    {
        qDebug() << "Error while loading the bounding box";
        exit(1);
    }

    _bot.x = wordsOfLine.at(0).toFloat();
    _bot.y = wordsOfLine.at(1).toFloat();
    _bot.z = wordsOfLine.at(2).toFloat();

    _top.x = wordsOfLine.at(3).toFloat();
    _top.y = wordsOfLine.at(4).toFloat();
    _top.z = wordsOfLine.at(5).toFloat();




////// The second line should contain the resolution and the number of voxels in the grid
    currentLine = stream.readLine();
    currentSizeRead += currentLine.size()+1; // +1 for the "\n" char
	
	// Skip comments
	if ( currentLine[0] == '#' )
	{
		while ( currentLine[0] == '#' )
		{
			currentLine = stream.readLine();
			currentSizeRead += currentLine.size()+1; // +1 for the "\n" char
		}
	}

    // Read each word separately
    wordsOfLine = currentLine.split( QRegExp("\\s+"), QString::SkipEmptyParts );

    // Checking for a valid line
    if ( wordsOfLine.size() != 2 )
    {
        qDebug() << "Error while loading the number of voxels";
        exit(1);
    }
    _res = wordsOfLine.at(0).toFloat();
    _nVoxels = wordsOfLine.at(1).toUInt();





////// The third line should contain the number of categories
    currentLine = stream.readLine();
    currentSizeRead += currentLine.size()+1; // +1 for the "\n" char
	
	// Skip comments
	if ( currentLine[0] == '#' )
	{
		while ( currentLine[0] == '#' )
		{
			currentLine = stream.readLine();
			currentSizeRead += currentLine.size()+1; // +1 for the "\n" char
		}
	}

    // Read each word separately
    wordsOfLine = currentLine.split( QRegExp("\\s+"), QString::SkipEmptyParts );

    // Checking for a valid line
    if ( wordsOfLine.size() != 1 )
    {
        qDebug() << "Error while loading the number of categories";
        exit(1);
    }
    _nCategories = wordsOfLine.at(0).toInt();

	
	
////// The fourth line should contain the categories benchmarks
    currentLine = stream.readLine();
    currentSizeRead += currentLine.size()+1; // +1 for the "\n" char
	
	// Skip comments
	if ( currentLine[0] == '#' )
	{
		while ( currentLine[0] == '#' )
		{
			currentLine = stream.readLine();
			currentSizeRead += currentLine.size()+1; // +1 for the "\n" char
		}
	}

    // Read each word separately
    wordsOfLine = currentLine.split( QRegExp("\\s+"), QString::SkipEmptyParts );

    if ( _nCategories > 1 )
    {
        // Checking for a valid line
        if ( wordsOfLine.size() != _nCategories-1 )
        {
            qDebug() << "Error while loading the categories benchmarks";
            exit(1);
        }

        for ( int i = 0 ; i < _nCategories-1 ; i++ )
        {
            _categoriesMarks.push_back( wordsOfLine.at(i).toInt() );
        }
    }

    else
    {
        // Checking for a valid line
        if ( wordsOfLine.size() != 1 )
        {
            qDebug() << "Error while loading the categories benchmarks";
            exit(1);
        }
    }

    return currentSizeRead;
}

void StepLoadFileLDF::readDataFile(QFile &f, int offset, bool little_endian)
{
    // Taking the offset into account
    f.seek(offset);
	
    // Get the future results
    CT_ResultGroup* outResultDensity;
    CT_ResultGroup* outResultHit;
    CT_ResultGroup* outResultTheoritical;
    CT_ResultGroup* outResultBefore;

    if ( _nCategories > 1 )
    {
        outResultBefore = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(_nCategories));
        outResultTheoritical = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(_nCategories+1));
        outResultHit = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(_nCategories+2));
        outResultDensity = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(_nCategories+3));
    }

    else
    {
        outResultBefore = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(0));
        outResultTheoritical = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(1));
        outResultHit = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(2));
        outResultDensity = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(3));
    }

    // Declaration des differentes grilles
    CT_RegularGridDouble*	densityGrid = new CT_RegularGridDouble(1, outResultDensity, _top, _bot, _res );
    CT_RegularGridInt*      hitGrid = new CT_RegularGridInt(2, outResultHit, _top, _bot, _res );
    CT_RegularGridInt*      theoriticalGrid = new CT_RegularGridInt(3, outResultTheoritical, _top, _bot, _res );
    CT_RegularGridInt*      beforeGrid = new CT_RegularGridInt(4, outResultBefore, _top, _bot, _res );
	
    QList< CT_RegularGridInt* > categoryHitGridList;
	if ( _nCategories > 1 )
	{
		for ( int j = 0 ; j < _nCategories ; j++ )
            categoryHitGridList.push_back( new CT_RegularGridInt(0, dynamic_cast<CT_ResultGroup*>(getOutResultList().at(j)), _top, _bot, _res ) );
	}

    // The position of the stream is now the begening of the grids description
    QTextStream stream ( &f );
    QString     currentLine;
    QStringList wordsOfLine;

    // For each voxel
    for ( int i = 0 ; i < _nVoxels ; i++ )
    {
        if ( stream.atEnd() )
        {
            qDebug() << "Reaching end of file while not having read all voxels";
            exit(1);
        }

        currentLine = stream.readLine();

        // Read each word separately
        wordsOfLine = currentLine.split( QRegExp("\\s+"), QString::SkipEmptyParts );
		
		// Skip comments
		if ( wordsOfLine.at(0) == "#" )
		{
			while ( wordsOfLine.at(0) == "#" )
			{
				currentLine = stream.readLine();
				wordsOfLine = currentLine.split( QRegExp("\\s+"), QString::SkipEmptyParts );
			}
				
		}

        // Checking for a valid line
        if ( _nCategories > 1 )
        {
            if ( wordsOfLine.size() != 4+_nCategories )
            {
                qDebug() << "Error while loading the file at voxel " << _nVoxels;
                exit(1);
            }
        }

        else
        {
            if ( wordsOfLine.size() != 4 )
            {
                qDebug() << "Error while loading the file at voxel " << _nVoxels;
                exit(1);
            }
        }

		densityGrid->setData( i, wordsOfLine.at(0).toDouble() );
		hitGrid->setData( i, wordsOfLine.at(1).toInt() );
		theoriticalGrid->setData( i, wordsOfLine.at(2).toInt() );
		beforeGrid->setData( i, wordsOfLine.at(3).toInt() );
		
        if ( _nCategories > 1 )
        {
            for ( int j = 0 ; j < _nCategories ; j++ )
            {
                categoryHitGridList[j]->setData(i, wordsOfLine.at(4+j).toInt() );
            }    
        }
    }

    // Setting the outputs result
    densityGrid->changeResult( outResultDensity );
    hitGrid->changeResult( outResultHit );
    theoriticalGrid->changeResult( outResultTheoritical );
    beforeGrid->changeResult( outResultBefore );

    if ( _nCategories > 1 )
    {
        for ( int i = 0 ; i < _nCategories ; i++ )
        {
            categoryHitGridList[i]->changeResult( dynamic_cast<CT_ResultGroup*>(getOutResultList().at(i)) );
        }
    }

    outResultDensity->setItemDrawable( densityGrid );
    outResultHit->setItemDrawable( hitGrid );
    outResultTheoritical->setItemDrawable( theoriticalGrid );
    outResultBefore->setItemDrawable( beforeGrid );

    if ( _nCategories > 1 )
    {
        for ( int i = 0 ; i < _nCategories ; i++ )
        {
            dynamic_cast<CT_ResultGroup*>(getOutResultList().at(i))->setItemDrawable( categoryHitGridList[i] );
			categoryHitGridList[i]->calculateMinMax();
        }
    }
	

	densityGrid->calculateMinMax();
	hitGrid->calculateMinMax();
	theoriticalGrid->calculateMinMax();
	beforeGrid->calculateMinMax();
}

/////////////////////// SLOTS ///////////////////////

bool StepLoadFileLDF::configure()
{
    if(!isRunning())
    {
        QString s = QFileDialog::getOpenFileName(0, tr("Ouvrir un fichier .ldf"),  getFilePath(), "LVox Density File (*.ldf)");

        if(!s.isEmpty())
        {
            if(getFilePath() != s)
            {
                QFileInfo info(s);

                _defaultOpenDirPath = info.dir().path();

                QSettings *settings = _data->getSettingsFile();

                if(settings != NULL)
                {
                    settings->beginGroup(getStepName());

                    settings->setValue("defaultOpenDirPath", _defaultOpenDirPath);

                    settings->endGroup();
                }

                setFilePath(s);
            }
        }
    }

    return true;
}


void StepLoadFileLDF::readFloat(float &outputFloat, QStringList &stringList, int nWord, bool *success, QString msgError)
{
    outputFloat = stringList[nWord].toFloat( success );
    if ( !success )
    {
        qDebug() << msgError;
        exit(0);
    }
}
