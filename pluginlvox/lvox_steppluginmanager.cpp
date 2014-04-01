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

#include "lvox_steppluginmanager.h"

#include "ct_stepseparator.h"
#include "ct_steploadfileseparator.h"
#include "ct_stepcanbeaddedfirstseparator.h"
#include "step/lvox_stepcomputelvoxgrids.h"
#include "step/lvox_steploadinfile.h"
#include "step/lvox_stepcombinedensitygrids.h"

LVOX_StepPluginManager::LVOX_StepPluginManager() : CT_AbstractStepPlugin()
{
}

LVOX_StepPluginManager::~LVOX_StepPluginManager()
{
}

bool LVOX_StepPluginManager::loadGenericsStep()
{
    CT_StepSeparator *sep = addNewSeparator(new CT_StepSeparator());

    // Steps with single scan input

    sep = addNewSeparator(new CT_StepSeparator());
    sep->addStep(new LVOX_StepLoadInFile(*createNewStepInitializeData(NULL)));
    sep->addStep( new LVOX_StepComputeLvoxGrids(*createNewStepInitializeData(NULL)) );
    sep->addStep( new LVOX_StepCombineDensityGrids(*createNewStepInitializeData(NULL)) );

	
    return true;
}

bool LVOX_StepPluginManager::loadOpenFileStep()
{
//    CT_StepLoadFileSeparator *sep = addNewSeparator(new CT_StepLoadFileSeparator("Lvox File (IN)"));
//    sep->addStep( new StepLoadLVoxInputFiles(*createNewStepInitializeData(NULL)) );

    return true;
}

bool LVOX_StepPluginManager::loadCanBeAddedFirstStep()
{
    CT_StepCanBeAddedFirstSeparator *sep = addNewSeparator(new CT_StepCanBeAddedFirstSeparator());

    sep->addStep(new LVOX_StepLoadInFile(*createNewStepInitializeData(NULL)));

    return true;
}

bool LVOX_StepPluginManager::loadActions()
{
    return true;
}

bool LVOX_StepPluginManager::loadExporters()
{
    return true;
}
