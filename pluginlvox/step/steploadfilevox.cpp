#include "steploadfilevox.h"

#include "ct_itemdrawable/ct_regulargridint.h"
#include "ct_itemdrawable/ct_regulargriddouble.h"

#include "ct_step/ct_stepinitializedata.h"
#include "ct_pointcloudstdvector.h"
#include "ct_pointcloudindexvector.h"
#include <limits>

#include <QFileDialog>
#include <QTextStream>
#include <QDebug>

StepLoadFileVOX::StepLoadFileVOX(CT_StepInitializeData &data) : CT_AbstractStepLoadFileInScene( data )
{
	
}

QString StepLoadFileVOX::getStepDescription() const
{
    return tr("Charge un fichier .vox");
}

CT_VirtualAbstractStep* StepLoadFileVOX::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new StepLoadFileVOX(dataInit);
}

QList<QString> StepLoadFileVOX::getFileExtensionAccepted() const
{
    QList<QString> list;
    list << ".vox";

    return list;
}

void StepLoadFileVOX::createOutResultModelListProtected()
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
            addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultGroup(), QList<CT_AbstractItemDrawable*>() << new CT_RegularGridDouble(), QString("DeltaInGridCategory").append(QString().number(i)) ) );
            addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultGroup(), QList<CT_AbstractItemDrawable*>() << new CT_RegularGridDouble(), QString("DeltaOutGridCategory").append(QString().number(i)) ) );
		}
	}

	if ( _fromPlugin )
	{
        addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultGroup(), QList<CT_AbstractItemDrawable*>() << new CT_RegularGridDouble(), "DeltaOutGrid"));
        addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultGroup(), QList<CT_AbstractItemDrawable*>() << new CT_RegularGridDouble(), "DeltaInGrid"));
	}
	
    addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultGroup(), QList<CT_AbstractItemDrawable*>() << new CT_RegularGridDouble(), "DeltaBeforeGrid"));
    addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultGroup(), QList<CT_AbstractItemDrawable*>() << new CT_RegularGridDouble(), "DeltaTheoGrid"));
	
    addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultGroup(), QList<CT_AbstractItemDrawable*>() << new CT_RegularGridInt(), "BeforeGrid"));
    addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultGroup(), QList<CT_AbstractItemDrawable*>() << new CT_RegularGridInt(), "TheoriticalGrid"));
    addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultGroup(), QList<CT_AbstractItemDrawable*>() << new CT_RegularGridInt(), "HitsGrid"));
    addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultGroup(), QList<CT_AbstractItemDrawable*>() << new CT_RegularGridDouble(), "DensityGrid"));
}

int StepLoadFileVOX::readHeaderFile(QFile &f)
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

	// The fifth line indicates wether the file comes from the computree plugin or the stand alone version
	currentLine = stream.readLine();
	currentSizeRead += currentLine.size()+1; // +1 for the "\n" char
    wordsOfLine = currentLine.split( QRegExp("\\s+"), QString::SkipEmptyParts );
	if ( wordsOfLine.size() != 1 )
	{
		qDebug() << "Error while loading the keyword";
		exit(1);
	}
	_fromPlugin = ( wordsOfLine.first() == "PLUGIN" );
	
    return currentSizeRead;
}

void StepLoadFileVOX::readDataFile(QFile &f, int offset, bool little_endian)
{
	if ( _fromPlugin )
		readLVoxPluginFile( f, offset, little_endian );
	
	else
		readLVoxStandAloneFile( f, offset, little_endian );
}

void StepLoadFileVOX::readLVoxStandAloneFile(QFile &f, int offset, bool little_endian)
{
    // Taking the offset into account
    f.seek(offset);
	
    // Get the future results
    CT_ResultGroup* outResultDensity;
    CT_ResultGroup* outResultHit;
    CT_ResultGroup* outResultTheoritical;
    CT_ResultGroup* outResultBefore;
    CT_ResultGroup* outResultDeltaTheo;
    CT_ResultGroup* outResultDeltaBefore;

    if ( _nCategories > 1 )
    {
        outResultDeltaBefore = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(3*_nCategories));
        outResultDeltaTheo = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(3*_nCategories+1));
        outResultBefore = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(3*_nCategories+2));
        outResultTheoritical = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(3*_nCategories+3));
        outResultHit = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(3*_nCategories+4));
        outResultDensity = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(3*_nCategories+5));
    }

    else
    {
        outResultDeltaBefore = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(0));
        outResultDeltaTheo = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(1));
        outResultBefore = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(2));
        outResultTheoritical = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(3));
        outResultHit = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(4));
        outResultDensity = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(5));
    }

    // Declaration des differentes grilles
    CT_RegularGridDouble*	densityGrid = new CT_RegularGridDouble(1, outResultDensity, _top, _bot, _res );
    CT_RegularGridInt*      hitGrid = new CT_RegularGridInt(2, outResultHit, _top, _bot, _res );
    CT_RegularGridInt*      theoriticalGrid = new CT_RegularGridInt(3, outResultTheoritical, _top, _bot, _res );
    CT_RegularGridInt*      beforeGrid = new CT_RegularGridInt(4, outResultBefore, _top, _bot, _res );
	CT_RegularGridDouble*	deltaTheoGrid = new CT_RegularGridDouble(5, outResultDeltaTheo, _top, _bot, _res );
	CT_RegularGridDouble*	deltaBeforeGrid = new CT_RegularGridDouble(6, outResultDeltaBefore, _top, _bot, _res );
	
    QList< CT_RegularGridInt* > categoryHitGridList;
	QList< CT_RegularGridDouble* > categoryDeltaInGridList;
	QList< CT_RegularGridDouble* > categoryDeltaOutGridList;
	if ( _nCategories > 1 )
	{
		for ( int j = 0 ; j < _nCategories ; j++ )
		{
            categoryHitGridList.push_back( new CT_RegularGridInt(0, dynamic_cast<CT_ResultGroup*>(getOutResultList().at(3*j)), _top, _bot, _res ) );
            categoryDeltaInGridList.push_back( new CT_RegularGridDouble(0, dynamic_cast<CT_ResultGroup*>(getOutResultList().at(3*j + 1)), _top, _bot, _res ) );
            categoryDeltaOutGridList.push_back( new CT_RegularGridDouble(0, dynamic_cast<CT_ResultGroup*>(getOutResultList().at(3*j + 2)), _top, _bot, _res ) );
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
			if ( wordsOfLine.size() != 6+3*_nCategories )
			{
				qDebug() << "Error while loading the file at voxel " << i;
				exit(1);
			}
		}

		else
		{
			if ( wordsOfLine.size() != 6 )
			{
				qDebug() << "Error while loading the file at voxel " << i;
				exit(1);
			}
		}

		densityGrid->setData( i, wordsOfLine.at(0).toDouble() );
		hitGrid->setData( i, wordsOfLine.at(1).toInt() );
		theoriticalGrid->setData( i, wordsOfLine.at(2).toInt() );
		beforeGrid->setData( i, wordsOfLine.at(3).toInt() );
		deltaTheoGrid->setData( i, wordsOfLine.at(4).toDouble() );
		deltaBeforeGrid->setData( i, wordsOfLine.at(5).toDouble() );
		
		if ( _nCategories > 1 )
		{
			for ( int j = 0 ; j < _nCategories ; j++ )
			{
				categoryHitGridList[j]->setData(i, wordsOfLine.at(6 + 3*j).toInt() );
				categoryDeltaInGridList[j]->setData(i, wordsOfLine.at(6 + 3*j + 1).toDouble() );
				categoryDeltaOutGridList[j]->setData(i, wordsOfLine.at(6 + 3*j + 2).toDouble() );
			}
		}
    }

    // Setting the outputs result
    densityGrid->changeResult( outResultDensity );
    hitGrid->changeResult( outResultHit );
    theoriticalGrid->changeResult( outResultTheoritical );
    beforeGrid->changeResult( outResultBefore );
	deltaTheoGrid->changeResult( outResultDeltaTheo );
	deltaBeforeGrid->changeResult( outResultDeltaBefore );
	
    if ( _nCategories > 1 )
    {
        for ( int i = 0 ; i < _nCategories ; i++ )
        {
            categoryHitGridList[i]->changeResult( dynamic_cast<CT_ResultGroup*>(getOutResultList().at( 3 * i)) );
            categoryDeltaInGridList[i]->changeResult( dynamic_cast<CT_ResultGroup*>(getOutResultList().at( 3 * i + 1)) );
            categoryDeltaOutGridList[i]->changeResult( dynamic_cast<CT_ResultGroup*>(getOutResultList().at( 3 * i + 2)) );
        }
    }

    outResultDensity->setItemDrawable( densityGrid );
    outResultHit->setItemDrawable( hitGrid );
    outResultTheoritical->setItemDrawable( theoriticalGrid );
    outResultBefore->setItemDrawable( beforeGrid );
	outResultDeltaTheo->setItemDrawable( deltaTheoGrid );
	outResultDeltaBefore->setItemDrawable( deltaBeforeGrid );

    if ( _nCategories > 1 )
    {
        for ( int i = 0 ; i < _nCategories ; i++ )
        {
            dynamic_cast<CT_ResultGroup*>(getOutResultList().at( 3 * i))->setItemDrawable( categoryHitGridList[i] );
			categoryHitGridList[i]->calculateMinMax();
			
            dynamic_cast<CT_ResultGroup*>(getOutResultList().at( 3 * i + 1))->setItemDrawable( categoryDeltaInGridList[i] );
			categoryDeltaInGridList[i]->calculateMinMax();
			
            dynamic_cast<CT_ResultGroup*>(getOutResultList().at( 3 * i + 2))->setItemDrawable( categoryDeltaOutGridList[i] );
			categoryDeltaOutGridList[i]->calculateMinMax();
        }
    }
	
	densityGrid->calculateMinMax();
	hitGrid->calculateMinMax();
	theoriticalGrid->calculateMinMax();
	beforeGrid->calculateMinMax();
	deltaTheoGrid->calculateMinMax();
	deltaBeforeGrid->calculateMinMax();
}

void StepLoadFileVOX::readLVoxPluginFile(QFile &f, int offset, bool little_endian)
{
    // Taking the offset into account
    f.seek(offset);
	
    // Get the future results
    CT_ResultGroup* outResultDensity;
    CT_ResultGroup* outResultHit;
    CT_ResultGroup* outResultTheoritical;
    CT_ResultGroup* outResultBefore;
    CT_ResultGroup* outResultDeltaTheo;
    CT_ResultGroup* outResultDeltaBefore;
    CT_ResultGroup* outResultDeltaIn;
    CT_ResultGroup* outResultDeltaOut;

    if ( _nCategories > 1 )
    {
        outResultDeltaOut = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(3*_nCategories));
        outResultDeltaIn = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(3*_nCategories+1));
        outResultDeltaBefore = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(3*_nCategories+2));
        outResultDeltaTheo = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(3*_nCategories+3));
        outResultBefore = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(3*_nCategories+4));
        outResultTheoritical = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(3*_nCategories+5));
        outResultHit = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(3*_nCategories+6));
        outResultDensity = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(3*_nCategories+7));
    }

    else
    {
        outResultDeltaOut = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(0));
        outResultDeltaIn = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(1));
        outResultDeltaBefore = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(2));
        outResultDeltaTheo = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(3));
        outResultBefore = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(4));
        outResultTheoritical = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(5));
        outResultHit = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(6));
        outResultDensity = dynamic_cast<CT_ResultGroup*>(getOutResultList().at(7));
    }

    // Declaration des differentes grilles
    CT_RegularGridDouble*	densityGrid = new CT_RegularGridDouble(1, outResultDensity, _top, _bot, _res );
    CT_RegularGridInt*      hitGrid = new CT_RegularGridInt(2, outResultHit, _top, _bot, _res );
    CT_RegularGridInt*      theoriticalGrid = new CT_RegularGridInt(3, outResultTheoritical, _top, _bot, _res );
    CT_RegularGridInt*      beforeGrid = new CT_RegularGridInt(4, outResultBefore, _top, _bot, _res );
	CT_RegularGridDouble*	deltaTheoGrid = new CT_RegularGridDouble(5, outResultDeltaTheo, _top, _bot, _res );
	CT_RegularGridDouble*	deltaBeforeGrid = new CT_RegularGridDouble(6, outResultDeltaBefore, _top, _bot, _res );
	CT_RegularGridDouble*	deltaInGrid = new CT_RegularGridDouble(7, outResultDeltaIn, _top, _bot, _res );
	CT_RegularGridDouble*	deltaOutGrid = new CT_RegularGridDouble(8, outResultDeltaOut, _top, _bot, _res );
	
    QList< CT_RegularGridInt* > categoryHitGridList;
	QList< CT_RegularGridDouble* > categoryDeltaInGridList;
	QList< CT_RegularGridDouble* > categoryDeltaOutGridList;
	if ( _nCategories > 1 )
	{
		for ( int j = 0 ; j < _nCategories ; j++ )
		{
            categoryHitGridList.push_back( new CT_RegularGridInt(0, dynamic_cast<CT_ResultGroup*>(getOutResultList().at(3*j)), _top, _bot, _res ) );
            categoryDeltaInGridList.push_back( new CT_RegularGridDouble(0, dynamic_cast<CT_ResultGroup*>(getOutResultList().at(3*j + 1)), _top, _bot, _res ) );
            categoryDeltaOutGridList.push_back( new CT_RegularGridDouble(0, dynamic_cast<CT_ResultGroup*>(getOutResultList().at(3*j + 2)), _top, _bot, _res ) );
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
			if ( wordsOfLine.size() != 8+3*_nCategories )
			{
				qDebug() << "Error while loading the file at voxel " << i;
				exit(1);
			}
		}

		else
		{
			if ( wordsOfLine.size() != 8 )
			{
				qDebug() << "Error while loading the file at voxel " << i;
				exit(1);
			}
		}

		densityGrid->setData( i, wordsOfLine.at(0).toDouble() );
		hitGrid->setData( i, wordsOfLine.at(1).toInt() );
		theoriticalGrid->setData( i, wordsOfLine.at(2).toInt() );
		beforeGrid->setData( i, wordsOfLine.at(3).toInt() );
		deltaTheoGrid->setData( i, wordsOfLine.at(4).toDouble() );
		deltaBeforeGrid->setData( i, wordsOfLine.at(5).toDouble() );
		deltaInGrid->setData( i, wordsOfLine.at(6).toDouble() );
		deltaOutGrid->setData( i, wordsOfLine.at(7).toDouble() );
		
		if ( _nCategories > 1 )
		{
			for ( int j = 0 ; j < _nCategories ; j++ )
			{
				categoryHitGridList[j]->setData(i, wordsOfLine.at(8 + 3*j).toInt() );
				categoryDeltaInGridList[j]->setData(i, wordsOfLine.at(8 + 3*j + 1).toDouble() );
				categoryDeltaOutGridList[j]->setData(i, wordsOfLine.at(8 + 3*j + 2).toDouble() );
			}
		}
    }

    // Setting the outputs result
    densityGrid->changeResult( outResultDensity );
    hitGrid->changeResult( outResultHit );
    theoriticalGrid->changeResult( outResultTheoritical );
    beforeGrid->changeResult( outResultBefore );
	deltaTheoGrid->changeResult( outResultDeltaTheo );
	deltaBeforeGrid->changeResult( outResultDeltaBefore );
	deltaInGrid->changeResult( outResultDeltaIn );
	deltaOutGrid->changeResult( outResultDeltaOut );
	
    if ( _nCategories > 1 )
    {
        for ( int i = 0 ; i < _nCategories ; i++ )
        {
            categoryHitGridList[i]->changeResult( dynamic_cast<CT_ResultGroup*>(getOutResultList().at( 3 * i)) );
            categoryDeltaInGridList[i]->changeResult( dynamic_cast<CT_ResultGroup*>(getOutResultList().at( 3 * i + 1)) );
            categoryDeltaOutGridList[i]->changeResult( dynamic_cast<CT_ResultGroup*>(getOutResultList().at( 3 * i + 2)) );
        }
    }

    outResultDensity->setItemDrawable( densityGrid );
    outResultHit->setItemDrawable( hitGrid );
    outResultTheoritical->setItemDrawable( theoriticalGrid );
    outResultBefore->setItemDrawable( beforeGrid );
	outResultDeltaTheo->setItemDrawable( deltaTheoGrid );
	outResultDeltaBefore->setItemDrawable( deltaBeforeGrid );
	outResultDeltaIn->setItemDrawable( deltaInGrid );
	outResultDeltaOut->setItemDrawable( deltaOutGrid );

    if ( _nCategories > 1 )
    {
        for ( int i = 0 ; i < _nCategories ; i++ )
        {
            dynamic_cast<CT_ResultGroup*>(getOutResultList().at( 3 * i))->setItemDrawable( categoryHitGridList[i] );
			categoryHitGridList[i]->calculateMinMax();
			
            dynamic_cast<CT_ResultGroup*>(getOutResultList().at( 3 * i + 1))->setItemDrawable( categoryDeltaInGridList[i] );
			categoryDeltaInGridList[i]->calculateMinMax();
			
            dynamic_cast<CT_ResultGroup*>(getOutResultList().at( 3 * i + 2))->setItemDrawable( categoryDeltaOutGridList[i] );
			categoryDeltaOutGridList[i]->calculateMinMax();
        }
    }
	
	densityGrid->calculateMinMax();
	hitGrid->calculateMinMax();
	theoriticalGrid->calculateMinMax();
	beforeGrid->calculateMinMax();
	deltaTheoGrid->calculateMinMax();
	deltaBeforeGrid->calculateMinMax();
	deltaInGrid->calculateMinMax();
	deltaOutGrid->calculateMinMax();
}

bool StepLoadFileVOX::configure()
{
    if(!isRunning())
    {
        QString s = QFileDialog::getOpenFileName(0, tr("Ouvrir un fichier .vox"),  getFilePath(), "Voxel density file (*.vox)");

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

void StepLoadFileVOX::readFloat(float &outputFloat, QStringList &stringList, int nWord, bool *success, QString msgError)
{
    outputFloat = stringList[nWord].toFloat( success );
    if ( !success )
    {
        qDebug() << msgError;
        exit(0);
    }
}
