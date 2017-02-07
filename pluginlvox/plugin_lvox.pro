COMPUTREE += ctlibio

CHECK_CAN_USE_OPENCV = 1

CT_PREFIX = ../../computreev3
include($${CT_PREFIX}/shared.pri)
include($${PLUGIN_SHARED_DIR}/include.pri)
include($${CT_PREFIX}/include_ct_library.pri)

contains ( QT_VERSION, "^5.*" ) {
    LIBS += -lQt5Concurrent
}

TARGET = plug_lvoxv2

HEADERS += $${PLUGIN_SHARED_INTERFACE_DIR}/interfaces.h \
    lvox_steppluginmanager.h \
    lvox_pluginentry.h \
    tools/lvox_distancevisitor.h \
    tools/lvox_countvisitor.h \
    tools/lvox_computehitsthread.h \
    tools/lvox_computetheoriticalsthread.h \
    tools/lvox_computebeforethread.h \
    step/lvox_stepcomputelvoxgrids.h \
    step/lvox_steploadinfile.h \
    tools/lvox_computedensitythread.h \
    step/lvox_stepcombinedensitygrids.h \
    step/lvox_stepcomputepad.h \
    step/lvox_stepinterpolatedensitygrid.h \
    step/lvox_stepcomputeprofile.h \
    step/lvox_stepndntgrids.h \
#    step/lvox_stepmergeinputs.h \
    step/lvox_stepexportcomputedgrids.h \
    tools/lvox_grid3dexporter.h \
#    step/lvox_stepimportcomputedgrids.h \
    step/lvox_stepexportmergedgrids.h \
#    step/lvox_stepimportmergedgrids.h \
    step/lvox_stepcomparegrids.h \
    urfm/step/lvox2_stepcombinedensitygrids.h \
    urfm/step/lvox2_stepcombinelvoxgrids.h \
    urfm/step/lvox2_stepcomputelvoxgrids.h \
    urfm/step/lvox2_stepexportcomputedgrids.h \
    urfm/tools/lvox2_computeactualbeamthread.h \
    urfm/tools/lvox2_computehitsthread.h \
    urfm/tools/lvox2_countvisitor.h \
    urfm/step/lvox2_stepcomputeheightprofile.h \
    urfm/step/lvox2_stepfiltergridbyradius.h \
    urfm/step/lvox2_steppreparepointcloud.h \
    tools/lvox_countwithlengththresholdvisitor.h \
    tools/lvox_computebeforewithlengththresholdthread.h \
    step/lvox_stepcomputeocclusionspace.h \
    mk/step/lvox3_stepcomputelvoxgrids.h \
    mk/tools/lvox3_computehits.h \
    mk/tools/lvox3_errorcode.h \
    mk/tools/lvox3_worker.h \
    mk/tools/lvox3_computetheoriticals.h \
    mk/tools/lvox3_computebefore.h \
    mk/tools/lvox3_computedensity.h \
    mk/tools/lvox3_computelvoxgridspreparator.h \
    mk/tools/lvox3_gridmode.h \
    mk/tools/lvox3_gridtype.h \
    mk/tools/lvox3_computeall.h \
    mk/tools/lvox3_gridtools.h \
    mk/tools/lvox3_grid3dwootraversalalgorithm.h \
    mk/tools/lvox3_rayboxintersectionmath.h \
    mk/tools/lvox3_grid3dvoxelvisitor.h \
    mk/tools/lvox3_grid3dvoxelvisitorcontext.h \
    mk/tools/lvox3_countvisitor.h \
    mk/tools/lvox3_distancevisitor.h \
    mk/view/loadfileconfiguration.h \
    mk/step/lvox3_steploadfiles.h \
    mk/step/lvox3_stepgenericcomputegrids.h \
    mk/view/genericcomputegridsconfiguration.h \
    mk/view/predefinedmapperconfiguration.h \
    mk/tools/lvox3_genericcompute.h \
    mk/tools/lvox3_genericconfiguration.h \
    mk/step/lvox3_stepcomputeprofiles.h \
    mk/view/computeprofilesconfiguration.h \
    mk/step/lvox3_stepcomputesky.h \
    mk/tools/lvox3_filtervoxelsbyzvaluesofraster.h

SOURCES += \
    lvox_pluginentry.cpp \
    lvox_steppluginmanager.cpp \
    tools/lvox_distancevisitor.cpp \
    tools/lvox_countvisitor.cpp \
    tools/lvox_computehitsthread.cpp \
    tools/lvox_computetheoriticalsthread.cpp \
    tools/lvox_computebeforethread.cpp \
    step/lvox_stepcomputelvoxgrids.cpp \
    step/lvox_steploadinfile.cpp \
    tools/lvox_computedensitythread.cpp \
    step/lvox_stepcombinedensitygrids.cpp \
    step/lvox_stepcomputepad.cpp \
    step/lvox_stepinterpolatedensitygrid.cpp \
    step/lvox_stepcomputeprofile.cpp \
    step/lvox_stepndntgrids.cpp \
#    step/lvox_stepmergeinputs.cpp \
    step/lvox_stepexportcomputedgrids.cpp \
    tools/lvox_grid3dexporter.cpp \
#    step/lvox_stepimportcomputedgrids.cpp \
#    step/lvox_stepimportmergedgrids.cpp \
    step/lvox_stepexportmergedgrids.cpp \
    step/lvox_stepcomparegrids.cpp \
    urfm/step/lvox2_stepcombinedensitygrids.cpp \
    urfm/step/lvox2_stepcombinelvoxgrids.cpp \
    urfm/step/lvox2_stepcomputelvoxgrids.cpp \
    urfm/step/lvox2_stepexportcomputedgrids.cpp \
    urfm/tools/lvox2_countvisitor.cpp \
    urfm/tools/lvox2_computeactualbeamthread.cpp \
    urfm/tools/lvox2_computehitsthread.cpp \
    urfm/step/lvox2_stepcomputeheightprofile.cpp \
    urfm/step/lvox2_stepfiltergridbyradius.cpp \
    urfm/step/lvox2_steppreparepointcloud.cpp \
    tools/lvox_countwithlengththresholdvisitor.cpp \
    tools/lvox_computebeforewithlengththresholdthread.cpp \
    step/lvox_stepcomputeocclusionspace.cpp \
    mk/step/lvox3_stepcomputelvoxgrids.cpp \
    mk/tools/lvox3_computehits.cpp \
    mk/tools/lvox3_worker.cpp \
    mk/tools/lvox3_computetheoriticals.cpp \
    mk/tools/lvox3_computebefore.cpp \
    mk/tools/lvox3_computedensity.cpp \
    mk/tools/lvox3_computelvoxgridspreparator.cpp \
    mk/tools/lvox3_computeall.cpp \
    mk/tools/lvox3_rayboxintersectionmath.cpp \
    mk/view/loadfileconfiguration.cpp \
    mk/step/lvox3_steploadfiles.cpp \
    mk/step/lvox3_stepgenericcomputegrids.cpp \
    mk/view/genericcomputegridsconfiguration.cpp \
    mk/view/predefinedmapperconfiguration.cpp \
    mk/tools/lvox3_genericcompute.cpp \
    mk/step/lvox3_stepcomputeprofiles.cpp \
    mk/view/computeprofilesconfiguration.cpp \
    mk/step/lvox3_stepcomputesky.cpp \
    mk/tools/lvox3_filtervoxelsbyzvaluesofraster.cpp

include(muParser/muparser.pri)

TRANSLATIONS += languages/pluginlvoxv2_en.ts \
                languages/pluginlvoxv2_fr.ts

INCLUDEPATH += .
INCLUDEPATH += ./step

FORMS += \
    mk/view/loadfileconfiguration.ui \
    mk/view/genericcomputegridsconfiguration.ui \
    mk/view/predefinedmapperconfiguration.ui \
    mk/view/computeprofilesconfiguration.ui

RESOURCES += \
    resources.qrc
