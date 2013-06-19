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

#include "steploadfileasc.h"

#include "ct_step/ct_stepinitializedata.h"
#include "ct_pointcloudstdvector.h"
#include "ct_pointcloudindexvector.h"
#include <limits>

#include <QFileDialog>
#include <QTextStream>
#include <QDebug>

StepLoadFileASC::StepLoadFileASC(CT_StepInitializeData &data) : CT_AbstractStepLoadFileInScene(data)
{
    QSettings *settings = data.getSettingsFile();

    if(settings != NULL)
    {
        settings->beginGroup(getStepName());

        _defaultOpenDirPath = settings->value("defaultOpenDirPath", ".").toString();

        settings->endGroup();
    }
}


QString StepLoadFileASC::getStepDescription() const
{
    return tr("Charge un fichier .asc");
}

CT_VirtualAbstractStep* StepLoadFileASC::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new StepLoadFileASC(dataInit);
}

QList<QString> StepLoadFileASC::getFileExtensionAccepted() const
{
    QList<QString> list;
    list << ".asc" << ".xyz";

    return list;
}

int StepLoadFileASC::readHeaderFile(QFile &f)
{
    // Il n'y a pas d'entete dans les fichier asc
    return 0;
}

void StepLoadFileASC::readDataFile(QFile &f, int offset, bool little_endian)
{
    // Creating the resulting point cloud and indices cloud
    CT_PointCloudStdVector *cloud = new CT_PointCloudStdVector(0);
    CT_PointCloudIndexVector *cloudIndex = new CT_PointCloudIndexVector(0);

    // Getting the file size
    qint64 fileSize = f.size();
    qint64 currentSizeRead = 0;

    float xmin = LONG_MAX;
    float ymin = LONG_MAX;
    float zmin = LONG_MAX;

    float xmax = LONG_MIN;
    float ymax = LONG_MIN;
    float zmax = LONG_MIN;

    QTextStream stream ( &f );
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
        wordsOfLine = currentLine.split( QRegExp("\\s+"), QString::SkipEmptyParts );

        // Checking for a valid line
        if ( wordsOfLine.size() != 6 )
        {
            qDebug() << "Error while loading the file \"" << f.fileName() << "\" at line " << nPoints;
            exit(1);
        }

        // Reads the attributes of the point
        readFloat( currentX, wordsOfLine, 0, &convertionToFloatSuccess, QString(QString("Unable to convert the ascii format to a floatting point at line ").append( QString().number(nPoints))) );
        readFloat( currentY, wordsOfLine, 1, &convertionToFloatSuccess, QString(QString("Unable to convert the ascii format to a floatting point at line ").append( QString().number(nPoints))) );
        readFloat( currentZ, wordsOfLine, 2, &convertionToFloatSuccess, QString(QString("Unable to convert the ascii format to a floatting point at line ").append( QString().number(nPoints))) );
        readFloat( currentIntensity, wordsOfLine, 3, &convertionToFloatSuccess, QString(QString("Unable to convert the ascii format to a floatting point at line ").append( QString().number(nPoints))) );

        // Checks for min and max
        if ( currentX < xmin )
            xmin = currentX;

        if ( currentY < ymin )
            ymin = currentY;

        if ( currentZ < zmin )
            zmin = currentZ;

        if ( currentX > xmax )
            xmax = currentX;

        if ( currentY > ymax )
            ymax = currentY;

        if ( currentZ > zmax )
            zmax = currentZ;

        // Add this point to the point cloud
        cloud->addPoint( createCtPoint(currentX, currentY, currentZ, currentIntensity) );
        cloudIndex->addIndex( nPoints );

        // Progress bar
        setProgress( currentSizeRead*100.0/fileSize );

        nPoints++;
    }

    createOutResult(cloud, cloudIndex, xmin, xmax, ymin, ymax, zmin, zmax);
}

/////////////////////// SLOTS ///////////////////////

bool StepLoadFileASC::configure()
{
    if(!isRunning())
    {
        QString s = QFileDialog::getOpenFileName(0, tr("Ouvrir un fichier .asc"),  getFilePath(), "Lidar Ascii File (*.asc);;Lidar Ascii File (*.xyz)s");

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


void StepLoadFileASC::readFloat(float &outputFloat, QStringList &stringList, int nWord, bool *success, QString msgError)
{
    outputFloat = stringList[nWord].toFloat( success );
    if ( !success )
    {
        qDebug() << msgError;
        exit(0);
    }
}
