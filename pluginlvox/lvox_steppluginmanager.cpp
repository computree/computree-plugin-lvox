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

//#include "step/steploadfileasc.h"                        // Step loading a ".asc" file
//#include "step/steploadlvoxinputfiles.h"                 // Step loading a ".in" file
//#include "step/steploadfileldf.h"                        // Step loading a ".ldf" file
//#include "step/steploadfilemdf.h"                        // Step loading a ".mdf" file
//#include "step/steploadfilevox.h"						 // Step loading a ".vox" file
//#include "step/steploadfilemox.h"						 // Step loading a ".mox" file

// #include "step/stepcomputedensityonly.h"                 // Step computing the density index and ONLY the density (no distances) of a single scan
//#include "step/stepcomputedensityonlymultiscan.h"        // Step computing the density index and ONLY the density (no distances) of several scans

#include "step/stepcomputedensityanddistances.h"         // Step computing the density index and the distances of a single scan
//#include "step/stepcomputedensityanddistancesmultiscan.h"// Step computing the density index and the distances of several scans

//#include "step/stepcomputevisibility.h"

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
//    sep->addStep( new StepComputeDensityOnly(*createNewStepInitializeData(NULL)) );
    sep->addStep( new StepComputeDensityAndDistances(*createNewStepInitializeData(NULL)) );

//    sep = addNewSeparator(new CT_StepSeparator());

//    // Steps with multiple scans input
//    sep->addStep( new StepComputeDensityOnlyMultiScan(*createNewStepInitializeData(NULL)) );
//    sep->addStep( new StepComputeDensityAndDistancesMultiScan(*createNewStepInitializeData(NULL)) );
	
//	sep = addNewSeparator(new CT_StepSeparator());
//	sep->addStep( new StepComputeVisibility(*createNewStepInitializeData(NULL)) );
	
    return true;
}

bool LVOX_StepPluginManager::loadOpenFileStep()
{
//    CT_StepLoadFileSeparator *sep = addNewSeparator(new CT_StepLoadFileSeparator("Lvox File (IN)"));
//    sep->addStep( new StepLoadLVoxInputFiles(*createNewStepInitializeData(NULL)) );

//    sep = addNewSeparator(new CT_StepLoadFileSeparator("Lidar Ascii File / LVox"));
//    sep->addStep( new StepLoadFileASC(*createNewStepInitializeData(NULL)) );

//    sep = addNewSeparator(new CT_StepLoadFileSeparator("LVox Density File (LDF)"));
//    sep->addStep( new StepLoadFileLDF(*createNewStepInitializeData(NULL)) );
	
//    sep = addNewSeparator(new CT_StepLoadFileSeparator("Merged Density File (MDF)"));
//    sep->addStep( new StepLoadFileMDF(*createNewStepInitializeData(NULL)) );
	
//    sep = addNewSeparator(new CT_StepLoadFileSeparator("LVox voxel File (VOX)"));
//    sep->addStep( new StepLoadFileVOX(*createNewStepInitializeData(NULL)) );
	
//    sep = addNewSeparator(new CT_StepLoadFileSeparator("LVox voxel File (MOX)"));
//    sep->addStep( new StepLoadFileMOX(*createNewStepInitializeData(NULL)) );

    return true;
}

bool LVOX_StepPluginManager::loadCanBeAddedFirstStep()
{
    return true;
}
