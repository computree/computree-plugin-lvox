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

#include "steploadfileldf.h"

// Inclusion of out models
#include "ct_itemdrawable/model/outModel/ct_outstandardgroupmodel.h"
#include "ct_itemdrawable/model/outModel/ct_outstandardgroupmodel.h"
#include "ct_itemdrawable/model/outModel/ct_outstandarditemdrawablemodel.h"
#include "ct_result/model/outModel/ct_outresultmodelgroup.h"

// Inclusion of standard result class
#include "ct_result/ct_resultgroup.h"

// Inclusion of used ItemDrawable classes
#include "ct_itemdrawable/ct_regulargridint.h"
#include "ct_itemdrawable/ct_regulargriddouble.h"

#include "ct_step/ct_stepinitializedata.h"
#include "ct_pointcloudstdvector.h"
#include "ct_pointcloudindexvector.h"
#include <limits>

#include <QFileDialog>
#include <QTextStream>
#include <QDebug>

// Alias for indexing out models
#define DEF_resultOut_rbg "rbg"
#define DEF_groupOut_gbg "gbg"
#define DEF_itemOut_ibg "ibg"
#define DEF_resultOut_rtg "rtg"
#define DEF_groupOut_gtg "gtg"
#define DEF_itemOut_itg "itg"
#define DEF_resultOut_rhg "rhg"
#define DEF_groupOut_ghg "ghg"
#define DEF_itemOut_ihg "ihg"
#define DEF_resultOut_rdg "rdg"
#define DEF_groupOut_gdg "gdg"
#define DEF_itemOut_idg "idg"

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
            CT_OutStandardGroupModel *groupOutModel_cati = new CT_OutStandardGroupModel(QString("g%1").arg(i), new CT_StandardItemGroup());
            CT_OutStandardItemDrawableModel *itemOutModel_cati = new CT_OutStandardItemDrawableModel(QString("i%1").arg(i), new CT_RegularGridInt(), QString("%1%2").arg(tr("HitCategory")).arg(i), QString("%1%2").arg(tr("HitCategory")).arg(i));
            groupOutModel_cati->addItem(itemOutModel_cati);

            CT_OutResultModelGroup *resultOutModel_cati = new CT_OutResultModelGroup(QString("r%1").arg(i), groupOutModel_cati, QString("%1%2").arg(tr("HitGridCategory")).arg(i), QString("%1%2").arg(tr("HitGridCategory")).arg(i));
            addOutResultModel(resultOutModel_cati);

        }
	}


    CT_OutStandardGroupModel *groupOutModel_gbg = new CT_OutStandardGroupModel(DEF_groupOut_gbg, new CT_StandardItemGroup(), tr("gbg"));
    CT_OutStandardItemDrawableModel *itemOutModel_ibg = new CT_OutStandardItemDrawableModel(DEF_itemOut_ibg, new CT_RegularGridInt(), tr("ibg"), tr("BeforeGrid"));
    groupOutModel_gbg->addItem(itemOutModel_ibg);

    CT_OutResultModelGroup *resultOutModel_rbg = new CT_OutResultModelGroup(DEF_resultOut_rbg, groupOutModel_gbg, tr("BeforeGrid"), tr("BeforeGrid"));
    addOutResultModel(resultOutModel_rbg);


    CT_OutStandardGroupModel *groupOutModel_gtg = new CT_OutStandardGroupModel(DEF_groupOut_gtg, new CT_StandardItemGroup(), tr("gtg"));
    CT_OutStandardItemDrawableModel *itemOutModel_itg = new CT_OutStandardItemDrawableModel(DEF_itemOut_itg, new CT_RegularGridInt(), tr("itg"), tr("TheoriticalGrid"));
    groupOutModel_gtg->addItem(itemOutModel_itg);

    CT_OutResultModelGroup *resultOutModel_rtg = new CT_OutResultModelGroup(DEF_resultOut_rtg, groupOutModel_gtg, tr("TheoriticalGrid"), tr("TheoriticalGrid"));
    addOutResultModel(resultOutModel_rtg);


    CT_OutStandardGroupModel *groupOutModel_ghg = new CT_OutStandardGroupModel(DEF_groupOut_ghg, new CT_StandardItemGroup(), tr("ghg"));
    CT_OutStandardItemDrawableModel *itemOutModel_ihg = new CT_OutStandardItemDrawableModel(DEF_itemOut_ihg, new CT_RegularGridInt(), tr("ihg"), tr("HitsGrid"));
    groupOutModel_ghg->addItem(itemOutModel_ihg);

    CT_OutResultModelGroup *resultOutModel_rhg = new CT_OutResultModelGroup(DEF_resultOut_rhg, groupOutModel_ghg, tr("HitsGrid"), tr("HitsGrid"));
    addOutResultModel(resultOutModel_rhg);


    CT_OutStandardGroupModel *groupOutModel_gdg = new CT_OutStandardGroupModel(DEF_groupOut_gdg, new CT_StandardItemGroup(), tr("gdg"));
    CT_OutStandardItemDrawableModel *itemOutModel_idg = new CT_OutStandardItemDrawableModel(DEF_itemOut_idg, new CT_RegularGridDouble(), tr("idg"), tr("DensityGrid"));
    groupOutModel_gdg->addItem(itemOutModel_idg);

    CT_OutResultModelGroup *resultOutModel_rdg = new CT_OutResultModelGroup(DEF_resultOut_rdg, groupOutModel_gdg, tr("DensityGrid"), tr("DensityGrid"));
    addOutResultModel(resultOutModel_rdg);
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

    // Get OUT results
    QList<CT_ResultGroup*> outResultList = getOutResultList();

    CT_ResultGroup* outResultBefore = outResultList.at(_nCategories);
    CT_ResultGroup* outResultTheoritical = outResultList.at(1 + _nCategories);
    CT_ResultGroup* outResultHit = outResultList.at(2 + _nCategories);
    CT_ResultGroup* outResultDensity = outResultList.at(3 + _nCategories);

    // Get the group model corresponding to DEF_groupOut_gbg
    CT_OutStandardGroupModel* groupOutModel_gbg = (CT_OutStandardGroupModel*)getOutModelForCreation(outResultBefore, DEF_groupOut_gbg);
    // Get the item model corresponding to DEF_itemOut_ibg
    CT_OutStandardItemDrawableModel* itemOutModel_ibg = (CT_OutStandardItemDrawableModel*)getOutModelForCreation(outResultBefore, DEF_itemOut_ibg);

    // Get the group model corresponding to DEF_groupOut_gtg
    CT_OutStandardGroupModel* groupOutModel_gtg = (CT_OutStandardGroupModel*)getOutModelForCreation(outResultTheoritical, DEF_groupOut_gtg);
    // Get the item model corresponding to DEF_itemOut_itg
    CT_OutStandardItemDrawableModel* itemOutModel_itg = (CT_OutStandardItemDrawableModel*)getOutModelForCreation(outResultTheoritical, DEF_itemOut_itg);

    // Get the group model corresponding to DEF_groupOut_ghg
    CT_OutStandardGroupModel* groupOutModel_ghg = (CT_OutStandardGroupModel*)getOutModelForCreation(outResultHit, DEF_groupOut_ghg);
    // Get the item model corresponding to DEF_itemOut_ihg
    CT_OutStandardItemDrawableModel* itemOutModel_ihg = (CT_OutStandardItemDrawableModel*)getOutModelForCreation(outResultHit, DEF_itemOut_ihg);

    // Get the group model corresponding to DEF_groupOut_gdg
    CT_OutStandardGroupModel* groupOutModel_gdg = (CT_OutStandardGroupModel*)getOutModelForCreation(outResultDensity, DEF_groupOut_gdg);
    // Get the item model corresponding to DEF_itemOut_idg
    CT_OutStandardItemDrawableModel* itemOutModel_idg = (CT_OutStandardItemDrawableModel*)getOutModelForCreation(outResultDensity, DEF_itemOut_idg);


     // Creation of different grids
    CT_RegularGridDouble*	densityGrid = new CT_RegularGridDouble(itemOutModel_idg, 1 + _nCategories, outResultDensity, _top, _bot, _res );
    CT_RegularGridInt*      hitGrid = new CT_RegularGridInt(itemOutModel_ihg, 2 + _nCategories, outResultHit, _top, _bot, _res );
    CT_RegularGridInt*      theoriticalGrid = new CT_RegularGridInt(itemOutModel_itg, 3 + _nCategories, outResultTheoritical, _top, _bot, _res );
    CT_RegularGridInt*      beforeGrid = new CT_RegularGridInt(itemOutModel_ibg, 4 + _nCategories, outResultBefore, _top, _bot, _res );
	
    QList< CT_RegularGridInt* > categoryHitGridList;
	if ( _nCategories > 1 )
	{
        for ( int i = 0 ; i < _nCategories ; i++ )
        {
            CT_ResultGroup* result = outResultList.at(i);
            CT_OutStandardItemDrawableModel* itemModel = (CT_OutStandardItemDrawableModel*)getOutModelForCreation(result, QString("i%1").arg(i));
            categoryHitGridList.push_back( new CT_RegularGridInt(itemModel, i+1, result, _top, _bot, _res ) );
        }
	}

    // The position of the stream is now the begening of the grids description
    QTextStream stream ( &f );
    QString     currentLine;
    QStringList wordsOfLine;

    // For each voxel
    for ( unsigned int i = 0 ; i < _nVoxels ; i++ )
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

    CT_StandardItemGroup* groupOut_gbg = new CT_StandardItemGroup(groupOutModel_gbg, 0, outResultBefore);
    groupOut_gbg->addItemDrawable(beforeGrid);
    outResultBefore->addGroup(groupOut_gbg);

    CT_StandardItemGroup* groupOut_gtg = new CT_StandardItemGroup(groupOutModel_gtg, 0, outResultTheoritical);
    groupOut_gtg->addItemDrawable(theoriticalGrid);
    outResultTheoritical->addGroup(groupOut_gtg);

    CT_StandardItemGroup* groupOut_ghg = new CT_StandardItemGroup(groupOutModel_ghg, 0, outResultHit);
    groupOut_ghg->addItemDrawable(hitGrid);
    outResultHit->addGroup(groupOut_ghg);

    CT_StandardItemGroup* groupOut_gdg = new CT_StandardItemGroup(groupOutModel_gdg, 0, outResultDensity);
    groupOut_gdg->addItemDrawable(densityGrid);
    outResultDensity->addGroup(groupOut_gdg);

    if ( _nCategories > 1 )
    {
        for ( int i = 0 ; i < _nCategories ; i++ )
        {
            CT_ResultGroup* result = outResultList.at(i);
            CT_OutStandardGroupModel* groupModel = (CT_OutStandardGroupModel*)getOutModelForCreation(result, QString("g%1").arg(i));

            CT_StandardItemGroup* group = new CT_StandardItemGroup(groupModel, 0, result);
            group->addItemDrawable(categoryHitGridList[i]);
            result->addGroup(group);

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
