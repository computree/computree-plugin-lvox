#include <QObject>
#include "lvox3_utils.h"
#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"

const LVOXGridDefinitionStruct LVOX3_Utils::LVOXGridDefinitions[] = {
   { Theoretic, DEF_Nt,      lvox::Grid3Di::staticGetType(), DEF_Nt, DEF_Nt, DEF_LVOX_GRD_NT, "isNt" },
   { Hits,      DEF_Ni,      lvox::Grid3Di::staticGetType(), DEF_Ni, DEF_Ni, DEF_LVOX_GRD_NI, "isNi"},
   { Blocked,   DEF_Nb,      lvox::Grid3Di::staticGetType(), DEF_Nb, DEF_Nt, DEF_LVOX_GRD_NB, "isNb" },
   { Density,   DEF_Density, lvox::Grid3Df::staticGetType(), DEF_Density, DEF_Density, DEF_LVOX_GRD_DENSITY, "isDensity"},
};

LVOX3_Utils::LVOX3_Utils()
{

}

void LVOX3_Utils::requireGrid(CT_InResultModelGroupToCopy *model, LVOXGridId id)
{
    const LVOXGridDefinitionStruct& def = LVOXGridDefinitions[id];

    model->addItemModel(DEF_SearchInGroup, def.name, def.type,
                        QObject::tr(def.disp.toStdString().c_str()));
    model->addItemAttributeModel(def.name, def.name + "FlagModelName",
                                 QList<QString>() << def.attr,
                                 CT_AbstractCategory::ANY,
                                 QObject::tr(def.attr_label.toStdString().c_str()));
}
