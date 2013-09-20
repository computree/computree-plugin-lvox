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

#ifndef STEPLOADLVOXINPUTFILES_H
#define STEPLOADLVOXINPUTFILES_H

#include "ct_step/abstract/ct_abstractsteploadfileinscene.h"
#include "ct_itemdrawable/ct_scene.h"

#include "ct_pointcloudstdvector.h"
#include "ct_pointcloudindexvector.h"

class StepLoadLVoxInputFiles : public CT_AbstractStepLoadFileInScene
{
    Q_OBJECT

public:
    // Constructeur de l'Ã©tape
    StepLoadLVoxInputFiles(CT_StepInitializeData &data);

    // Description de l'etape (ce qu'elle fait), apparait dans la dialog box
    QString getStepDescription() const;

    // Renvoie une nouvelle instance de cette etape
    CT_VirtualAbstractStep* createNewInstance(CT_StepInitializeData &dataInit);

    // Renvoie une liste contenant les extensions de fichiers que l'etape sait ouvrir
    QList<QString>          getFileExtensionAccepted() const;

    // On redefinit la sortie qui n'est plus celle par defaut (une seule CT_Scene) mais plusieurs scenes et plusieurs scanners
    virtual void createOutResultModelListProtected();

protected:

    // Lit l'entete d'un fichier donne en parametre
    int     readHeaderFile  (QFile &f);

    // Lit le reste du fichier (en parametre), offset est le nombre de caracteres deja lus
    void    readDataFile    (QFile &f, int offset, bool little_endian = false);

private :
    // Renvoie la categorie correspondante a l'intensite en entree
    int getScanType ( QString scanType );

    // Renvoie un nuage de point et un nuage d'indices a partir d'un fichier .asc
    void createCloudsFromASCFile( QString fileName, CT_PointCloudStdVector*& outputCloud, CT_PointCloudIndexVector*& outputIndexCloud );

    // Lit le mot numero nWord dans la liste de strings stringlist. Si l'operation echou, on quitte avec un message d'erreur
    void readFloat( float& outputFloat, QStringList& stringList, int nWord, bool* success, QString msgError );

private:
    QString _defaultOpenDirPath;    // Repertoire par defaut ou aller chercher le fichier (repertoire ouvert initialement dans la boite de dialogue)
    float _xMin;                    // bottom point de la BBox de Lvox
    float _yMin;
    float _zMin;

    float _xMax;                    // top point de la BBox de Lvox
    float _yMax;
    float _zMax;

public slots:
    // De maniÃ¨re gÃ©nÃ©rale cette mÃ©thode devrait lire le nom du fichier Ã  ouvrir Ã  partir d'une boite de dialogue
    bool configure();
};

#endif // STEPLOADLVOXINPUTFILES_H
