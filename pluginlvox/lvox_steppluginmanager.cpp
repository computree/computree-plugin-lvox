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
//#include "step/lvox_stepmergeinputs.h"
#include "step/lvox_stepcomputepad.h"
#include "step/lvox_stepinterpolatedensitygrid.h"
#include "step/lvox_stepcomputeprofile.h"
#include "step/lvox_stepndntgrids.h"
#include "step/lvox_stepexportcomputedgrids.h"
//#include "step/lvox_stepimportcomputedgrids.h"
#include "step/lvox_stepexportmergedgrids.h"
//#include "step/lvox_stepimportmergedgrids.h"
#include "step/lvox_stepcomparegrids.h"


#include "urfm/step/lvox2_stepcomputelvoxgrids.h"
#include "urfm/step/lvox2_stepcombinedensitygrids.h"
#include "urfm/step/lvox2_stepexportcomputedgrids.h"
#include "urfm/step/lvox2_stepcomputeheightprofile.h"
#include "urfm/step/lvox2_stepfiltergridbyradius.h"
#include "urfm/step/lvox2_steppreparepointcloud.h"


#include "ct_global/ct_context.h"
#include "ct_categories/ct_stdcategory.h"


LVOX_StepPluginManager::LVOX_StepPluginManager() : CT_AbstractStepPlugin()
{
    PS_CATEGORY_MANAGER->registerCategory(new CT_StdCategory("LVOX_GRD_NI", QList<QString>() << CT_AbstractCategory::DATA_VALUE, "Ni"));
    PS_CATEGORY_MANAGER->registerCategory(new CT_StdCategory("LVOX_GRD_NB", QList<QString>() << CT_AbstractCategory::DATA_VALUE, "Nb"));
    PS_CATEGORY_MANAGER->registerCategory(new CT_StdCategory("LVOX_GRD_NT", QList<QString>() << CT_AbstractCategory::DATA_VALUE, "Nt"));
    PS_CATEGORY_MANAGER->registerCategory(new CT_StdCategory("LVOX_GRD_NTA", QList<QString>() << CT_AbstractCategory::DATA_VALUE, "Nta"));
    PS_CATEGORY_MANAGER->registerCategory(new CT_StdCategory("LVOX_GRD_DENSITY", QList<QString>() << CT_AbstractCategory::DATA_VALUE, "Density"));
}

LVOX_StepPluginManager::~LVOX_StepPluginManager()
{
}

bool LVOX_StepPluginManager::loadGenericsStep()
{

    addNewVoxelsStep<LVOX_StepLoadInFile>(QObject::tr("LVOX"));
    addNewVoxelsStep<LVOX_StepComputeLvoxGrids>(QObject::tr("LVOX"));
    addNewVoxelsStep<LVOX_StepExportComputedGrids>(QObject::tr("LVOX"));
//    addNewVoxelsStep<LVOX_StepImportComputedGrids>(QObject::tr("LVOX"));
//    addNewVoxelsStep<LVOX_StepMergeInputs>(QObject::tr("LVOX"));
    addNewVoxelsStep<LVOX_StepCombineDensityGrids>(QObject::tr("LVOX"));
    addNewVoxelsStep<LVOX_StepExportMergedGrids>(QObject::tr("LVOX"));
//    addNewVoxelsStep<LVOX_StepImportMergedGrids>(QObject::tr("LVOX"));
    addNewVoxelsStep<LVOX_StepInterpolateDensityGrids>(QObject::tr("LVOX"));
    addNewVoxelsStep<LVOX_StepNdNtGrids>(QObject::tr("LVOX"));
    addNewVoxelsStep<LVOX_StepComputeProfile>(QObject::tr("LVOX"));
    addNewVoxelsStep<LVOX_StepComputePAD>(QObject::tr("LVOX"));
    addNewVoxelsStep<LVOX_StepCompareGrids>(QObject::tr("LVOX"));

    addNewVoxelsStep<LVOX2_StepComputeLvoxGrids>(QObject::tr("LVOX2"));
    addNewVoxelsStep<LVOX2_StepCombineDensityGrids>(QObject::tr("LVOX2"));
    addNewVoxelsStep<LVOX2_StepExportComputedGrids>(QObject::tr("LVOX2"));
    addNewVoxelsStep<LVOX2_StepComputeHeightProfile>(QObject::tr("LVOX2"));
    addNewVoxelsStep<LVOX2_StepFilterGridByRadius>(QObject::tr("LVOX2"));
    addNewVoxelsStep<LVOX2_StepPreparePointCloud>(QObject::tr("LVOX2"));

    return true;
}

bool LVOX_StepPluginManager::loadOpenFileStep()
{

    return true;
}

bool LVOX_StepPluginManager::loadCanBeAddedFirstStep()
{
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

bool LVOX_StepPluginManager::loadReaders()
{
    return true;
}
