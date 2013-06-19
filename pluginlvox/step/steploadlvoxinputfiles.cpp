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

#include "steploadlvoxinputfiles.h"

#include "ct_step/ct_stepinitializedata.h"

#include "ct_itemdrawable/ct_scanner.h"
#include "ct_itemdrawable/ct_scene.h"

#include <QFileDialog>
#include <QTextStream>
#include <QDebug>

#define UNKNOWN 0
#define ILRIS3D 1
#define ZF 2
#define UNIVERSAL 3

StepLoadLVoxInputFiles::StepLoadLVoxInputFiles(CT_StepInitializeData &data) : CT_AbstractStepLoadFileInScene(data)
{
    // On recupere les parametres de l'étape fournis par Qt
    //
    // Position de la fenetre a l'ecran par exemple
    QSettings *settings = data.getSettingsFile();

    if(settings != NULL)
    {
        settings->beginGroup( getStepName() );

        _defaultOpenDirPath = settings->value("defaultOpenDirPath", ".").toString();

        settings->endGroup();
    }
}

QString StepLoadLVoxInputFiles::getStepDescription() const
{
    return tr("Charge un fichier .in");
}

CT_VirtualAbstractStep* StepLoadLVoxInputFiles::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new StepLoadLVoxInputFiles(dataInit);
}

QList<QString> StepLoadLVoxInputFiles::getFileExtensionAccepted() const
{
    QList<QString> list;
    list << ".in";

    return list;
}

void StepLoadLVoxInputFiles::createOutResultModelListProtected()
{
    addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultList(), QList<CT_AbstractItemDrawable*>() << new CT_Scene(), "SceneFromScanners"));
    addOutResultModel(new CT_ResultModelDefinedType(new CT_ResultList(), QList<CT_AbstractItemDrawable*>() << new CT_Scanner(), "ScannerFromInput"));
}

int StepLoadLVoxInputFiles::readHeaderFile(QFile &f)
{
    // Un fichier input de lvox ne comporte pas d'entete
    return 0;
}

void StepLoadLVoxInputFiles::readDataFile(QFile &f, int offset, bool little_endian)
{
    // On recupere les resultats de sortie pour pouvoir y ajouter les scanners et les scenes charges
    CT_ResultList* outputScenesList = (CT_ResultList*)getOutResultList().first();
    CT_ResultList* outputScannersList = (CT_ResultList*)getOutResultList().at(1);

    // Declaration de la liste de scanners en sortie
    QList< CT_Scanner* > scannersList;

    // Declaration de la liste de scenes en sortie
    QList< CT_Scene* > scenesList;

    // Reading tools
    QTextStream stream ( &f );
    QString     currentLine;
    QStringList wordsOfLine;
    bool        convertionToFloatSuccess;

    // Lecture de la bounding box de LVox
    currentLine = stream.readLine();

    // Read each word separately
    wordsOfLine = currentLine.split( QRegExp("\\s+"), QString::SkipEmptyParts );

    // Checking for a valid bounding box
    if ( wordsOfLine.size() != 6 )
    {
        qDebug() << "Error while loading the input file : bad description of bounding box";
        exit(1);
    }

    readFloat( _xMin, wordsOfLine, 0, &convertionToFloatSuccess, QString("Unable to convert the ascii format to a floatting point while reading the bounding box") );
    readFloat( _yMin, wordsOfLine, 1, &convertionToFloatSuccess, QString("Unable to convert the ascii format to a floatting point while reading the bounding box") );
    readFloat( _zMin, wordsOfLine, 2, &convertionToFloatSuccess, QString("Unable to convert the ascii format to a floatting point while reading the bounding box") );
    readFloat( _xMax, wordsOfLine, 3, &convertionToFloatSuccess, QString("Unable to convert the ascii format to a floatting point while reading the bounding box") );
    readFloat( _yMax, wordsOfLine, 4, &convertionToFloatSuccess, QString("Unable to convert the ascii format to a floatting point while reading the bounding box") );
    readFloat( _zMax, wordsOfLine, 5, &convertionToFloatSuccess, QString("Unable to convert the ascii format to a floatting point while reading the bounding box") );

    qint64      currentScanID = 0;

    // Tant que le fichier n'a pas ete lu entierement
    while( !stream.atEnd() )
    {
        // Declaration d'un pointeur vers un scanner qui sera attache aux resultats de sortie
        CT_Scanner* currentScan;

        // Lecture ligne par ligne de chaque scanner et de ses parametres
        QString     dataFile;
        QString     scanType;
        float       positionX, positionY, positionZ;
        float       hRes, vRes;
        float       hFov, vFov;
        float       initialTheta, initialPhi;

        // Read the scan file, scan type and position
        currentLine = stream.readLine();
        wordsOfLine = currentLine.split( QRegExp("\\s+"), QString::SkipEmptyParts );
        dataFile = wordsOfLine[0];
        scanType = wordsOfLine[1];

        readFloat( positionX, wordsOfLine, 2, &convertionToFloatSuccess, QString(QString("Unable to convert the ascii format to a floatting point while reading the position of the ").append(QString().number(currentScanID).append(QString("th scanner")))) );
        readFloat( positionY, wordsOfLine, 3, &convertionToFloatSuccess, QString(QString("Unable to convert the ascii format to a floatting point while reading the position of the ").append(QString().number(currentScanID).append(QString("th scanner")))) );
        readFloat( positionZ, wordsOfLine, 4, &convertionToFloatSuccess, QString(QString("Unable to convert the ascii format to a floatting point while reading the position of the ").append(QString().number(currentScanID).append(QString("th scanner")))) );

        // Convertion from floats to Qvector3D
        QVector3D position(positionX, positionY, positionZ);

        // Each type of scanner leads to different loading
        switch ( getScanType( scanType ) )
        {
            case ILRIS3D :
            {
                // Check validity of a n ILRIS-3D description
                if ( wordsOfLine.size() != 12 )
                {
                    qDebug() <<"Bad definition of a ILRIS-3D scanner at " << currentScanID << "th scanner";
                    exit(1);
                }

                // Read specific arguments for an ILRIS-3D scanner
                float targetX, targetY, targetZ;

                // Read ILRIS3D scanner parameters
                readFloat( targetX, wordsOfLine, 5, &convertionToFloatSuccess, QString(QString("Unable to convert the ascii format to a floatting point while reading target of the ").append(QString().number(currentScanID).append(QString("th scanner")))) );
                readFloat( targetY, wordsOfLine, 6, &convertionToFloatSuccess, QString(QString("Unable to convert the ascii format to a floatting point while reading target of the ").append(QString().number(currentScanID).append(QString("th scanner")))) );
                readFloat( targetZ, wordsOfLine, 7, &convertionToFloatSuccess, QString(QString("Unable to convert the ascii format to a floatting point while reading target angle of the ").append(QString().number(currentScanID).append(QString("th scanner")))) );
                readFloat( hRes, wordsOfLine, 8, &convertionToFloatSuccess, QString(QString("Unable to convert the ascii format to a floatting point while reading horizontal resolution of the ").append(QString().number(currentScanID).append(QString("th scanner")))) );
                readFloat( vRes, wordsOfLine, 9, &convertionToFloatSuccess, QString(QString("Unable to convert the ascii format to a floatting point while reading vertical resolution of the ").append(QString().number(currentScanID).append(QString("th scanner")))) );
                readFloat( hFov, wordsOfLine, 10, &convertionToFloatSuccess, QString(QString("Unable to convert the ascii format to a floatting point while reading horizontal field of view of the ").append(QString().number(currentScanID).append(QString("th scanner")))) );
                readFloat( vFov, wordsOfLine, 11, &convertionToFloatSuccess, QString(QString("Unable to convert the ascii format to a floatting point while reading vertical field of view of the ").append(QString().number(currentScanID).append(QString("th scanner")))) );

                // Convertion from degres to radians
                hFov = hFov * DEG2RAD;
                vFov = vFov * DEG2RAD;
                hRes = hRes * DEG2RAD;
                vRes = vRes * DEG2RAD;

                // Check validity of resolution and field of view
                assert( hRes > 0 && vRes > 0 && hFov > 0 && vFov > 0 );

                // Computation of the initial angles
                QVector3D direction(targetX - positionX, targetY - positionY, targetZ - positionZ );
                initialTheta = atan2 ( direction.y(), direction.x() ) - (hFov/2.0);
                initialPhi = acos ( direction.z() / direction.length() ) - (vFov/2.0);

                qDebug() << "initial theta = " << initialTheta << " et initial phi = " << initialPhi;
                qDebug() << "En degres : initial theta = " << initialTheta*RAD2DEG << " et initial phi = " << initialPhi*RAD2DEG;

                // Creating the new scan in radians (last parameter equals true)
                // The scan is considered not to be clockwise
                currentScan = new CT_Scanner(currentScanID, outputScannersList, currentScanID, position, QVector3D(0,0,1), hFov, vFov, hRes, vRes, initialTheta, initialPhi, false, true );

                // Adding the scanner to the result list
                scannersList.push_back( currentScan );
                break;
            }

            case ZF :
            {
                // Check validity of a Z+F description
                if ( wordsOfLine.size() != 11 )
                {
                    qDebug() <<"Bad definition of a Z+F scanner at " << currentScanID << "th scanner";
                    exit(1);
                }

                // Read specific arguments for a Z+F scanner
                float lastTheta, lastPhi;

                // Read Z+F scanner parameters
                readFloat( hRes, wordsOfLine, 5, &convertionToFloatSuccess, QString(QString("Unable to convert the ascii format to a floatting point while reading horizontal resolution of the ").append(QString().number(currentScanID).append(QString("th scanner")))) );
                readFloat( vRes, wordsOfLine, 6, &convertionToFloatSuccess, QString(QString("Unable to convert the ascii format to a floatting point while reading vertical resolution of the ").append(QString().number(currentScanID).append(QString("th scanner")))) );
                readFloat( initialTheta, wordsOfLine, 7, &convertionToFloatSuccess, QString(QString("Unable to convert the ascii format to a floatting point while reading initial theta angle of the ").append(QString().number(currentScanID).append(QString("th scanner")))) );
                readFloat( lastTheta, wordsOfLine, 8, &convertionToFloatSuccess, QString(QString("Unable to convert the ascii format to a floatting point while reading last theta angle of the ").append(QString().number(currentScanID).append(QString("th scanner")))) );
                readFloat( initialPhi, wordsOfLine, 9, &convertionToFloatSuccess, QString(QString("Unable to convert the ascii format to a floatting point while reading initial phi angle of view of the ").append(QString().number(currentScanID).append(QString("th scanner")))) );
                readFloat( lastPhi, wordsOfLine, 10, &convertionToFloatSuccess, QString(QString("Unable to convert the ascii format to a floatting point while reading last phi angle of view of the ").append(QString().number(currentScanID).append(QString("th scanner")))) );

                // Calculating the fields of view
                hFov = lastTheta - initialTheta;
                vFov = lastPhi - initialPhi;

                // Check validity of resolution and fields of view
                assert( hRes > 0 && vRes > 0 && hFov > 0 && vFov > 0 );

                // Creating the new scan in degres
                // The scan is considered not to be clockwise
                currentScan = new CT_Scanner(currentScanID, outputScannersList, currentScanID, position, QVector3D(0,0,1), hFov, vFov, hRes, vRes, initialTheta, initialPhi, false, false );

                // Adding the scanner to the result list
                scannersList.push_back( currentScan );
                break;
            }

            case UNIVERSAL :
            {
                // Check validity of a UNIVERSAL scanner description
                if ( wordsOfLine.size() != 11 )
                {
                    qDebug() <<"Bad definition of a UNIVERSAL scanner at " << currentScanID << "th scanner";
                    exit(1);
                }

                // Read universal scanner parameters
                readFloat( hRes, wordsOfLine, 5, &convertionToFloatSuccess, QString(QString("Unable to convert the ascii format to a floatting point while reading horizontal resolution of the ").append(QString().number(currentScanID).append(QString("th scanner")))) );
                readFloat( vRes, wordsOfLine, 6, &convertionToFloatSuccess, QString(QString("Unable to convert the ascii format to a floatting point while reading vertical resolution of the ").append(QString().number(currentScanID).append(QString("th scanner")))) );
                readFloat( initialTheta, wordsOfLine, 7, &convertionToFloatSuccess, QString(QString("Unable to convert the ascii format to a floatting point while reading initial theta angle of the ").append(QString().number(currentScanID).append(QString("th scanner")))) );
                readFloat( initialPhi, wordsOfLine, 8, &convertionToFloatSuccess, QString(QString("Unable to convert the ascii format to a floatting point while reading initial phi angle of the ").append(QString().number(currentScanID).append(QString("th scanner")))) );
                readFloat( hFov, wordsOfLine, 9, &convertionToFloatSuccess, QString(QString("Unable to convert the ascii format to a floatting point while reading horizontal field of view of the ").append(QString().number(currentScanID).append(QString("th scanner")))) );
                readFloat( vFov, wordsOfLine, 10, &convertionToFloatSuccess, QString(QString("Unable to convert the ascii format to a floatting point while reading vertical field of view of the ").append(QString().number(currentScanID).append(QString("th scanner")))) );


                // Creating the new scan in degres
                // The scan is considered not to be clockwise
                currentScan = new CT_Scanner(currentScanID, outputScannersList, currentScanID, position, QVector3D(0,0,1), hFov, vFov, hRes, vRes, initialTheta, initialPhi, false, false );

                // Adding the scanner to the result list
                scannersList.push_back( currentScan );
				break;
            }

            case UNKNOWN :
            {
                qDebug() << "Unrecognised scanner type : " << scanType ;
                exit(1);
                break;
            }

            default :
            {
                qDebug() << "Could not determine type of scanner";
                exit(1);
                break;
            }
        }

        // Creating the associated scene and attach it to the output result
        CT_PointCloudStdVector* currentCloud = NULL;
        CT_PointCloudIndexVector* currentIndexCloud = NULL;
        createCloudsFromASCFile( dataFile, currentCloud, currentIndexCloud );

        CT_Scene* currentScene = new CT_Scene(currentScanID, *outputScenesList, currentCloud, currentIndexCloud, new CT_AxisAlignedBoundingBox(QVector3D(_xMin, _yMin, _zMin), QVector3D(_xMax, _yMax, _zMax) ) );

        // Adding the scene to the sceneslist
        scenesList.push_back( currentScene );

        // Increment the number of scanner imported
        currentScanID++;
    }

    outputScannersList->setList( (QList<CT_AbstractItemDrawable*>&)scannersList );
    outputScenesList->setList( (QList<CT_AbstractItemDrawable*>&)scenesList );
}

int StepLoadLVoxInputFiles::getScanType(QString scanType)
{
    if ( scanType == "ILRIS-3D" )
        return ILRIS3D;

    else if ( scanType == "Z+F" )
        return ZF;

    else if ( scanType == "UNIVERSAL")
		return UNIVERSAL;

    else
        return UNKNOWN;
}

void StepLoadLVoxInputFiles::createCloudsFromASCFile(QString fileName, CT_PointCloudStdVector*& outputCloud, CT_PointCloudIndexVector*& outputIndexCloud)
{
    assert( outputCloud == NULL && outputIndexCloud == NULL );

    // Creating th resulting point cloud and indices cloud
    outputCloud = new CT_PointCloudStdVector(0);
    outputIndexCloud = new CT_PointCloudIndexVector(0);

    // Opening the file containing the scene
    QFile dataFile( fileName );
    dataFile.open(QIODevice::ReadOnly);

    // Getting the file size
    qint64 fileSize = dataFile.size();
    qint64 currentSizeRead = 0;

    QTextStream stream ( &dataFile );
    QString     currentLine;
    QStringList wordsOfLine;

    int nPoints = 0;
    float currentX, currentY, currentZ, currentIntensity;

    bool convertionToFloatSuccess;

    // While we did not reached the end of file
    while( !stream.atEnd() )
    {
        // Read the currentLine
        currentLine = stream.readLine();
        currentSizeRead += currentLine.size();

        // Read each word separately
        wordsOfLine = currentLine.split(" ", QString::SkipEmptyParts );

        // Checking for a valid line
        if ( wordsOfLine.size() != 6 )
        {
            qDebug() << "Error while loading the file \"" << fileName << "\" at line " << nPoints;
            exit(1);
        }

//        // Reads the attributes of the point
        readFloat( currentX, wordsOfLine, 0, &convertionToFloatSuccess, QString(QString("Unable to convert the ascii format to a floatting point at line ").append( QString().number(nPoints) )) );
        readFloat( currentY, wordsOfLine, 1, &convertionToFloatSuccess, QString(QString("Unable to convert the ascii format to a floatting point at line ").append( QString().number(nPoints) )) );
        readFloat( currentZ, wordsOfLine, 2, &convertionToFloatSuccess, QString(QString("Unable to convert the ascii format to a floatting point at line ").append( QString().number(nPoints) )) );
        readFloat( currentIntensity, wordsOfLine, 3, &convertionToFloatSuccess, QString(QString("Unable to convert the ascii format to a floatting point at line ").append( QString().number(nPoints) )) );

        // Add this point to the point cloud
        outputCloud->addPoint( createCtPoint(currentX, currentY, currentZ, currentIntensity) );
        outputIndexCloud->addIndex( nPoints );

        // Progress bar
        setProgress( currentSizeRead*100.0/fileSize );

        nPoints++;
    }

    // Closing file
    dataFile.close();
}

/////////////////////// SLOTS ///////////////////////

bool StepLoadLVoxInputFiles::configure()
{
    if(!isRunning())
    {
        QString s = QFileDialog::getOpenFileName(0, tr("Ouvrir un fichier .in"),  getFilePath(), "LVox File (*.in)");

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

void StepLoadLVoxInputFiles::readFloat(float &outputFloat, QStringList &stringList, int nWord, bool *success, QString msgError)
{
    outputFloat = stringList[nWord].toFloat( success );
    if ( !success )
    {
        qDebug() << msgError;
        exit(0);
    }
}
