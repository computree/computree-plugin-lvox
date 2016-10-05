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
    urfm/step/lvox2_stepcomputelvoxgrids.h \
    urfm/step/lvox2_stepexportcomputedgrids.h \
    urfm/tools/lvox2_computeactualbeamthread.h \
    urfm/tools/lvox2_countvisitor.h \
    urfm/step/lvox2_stepcomputeheightprofile.h \
    urfm/step/lvox2_stepfiltergridbyradius.h \
    urfm/step/lvox2_steppreparepointcloud.h

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
    urfm/step/lvox2_stepcomputelvoxgrids.cpp \
    urfm/step/lvox2_stepexportcomputedgrids.cpp \
    urfm/tools/lvox2_countvisitor.cpp \
    urfm/tools/lvox2_computeactualbeamthread.cpp \
    urfm/step/lvox2_stepcomputeheightprofile.cpp \
    urfm/step/lvox2_stepfiltergridbyradius.cpp \
    urfm/step/lvox2_steppreparepointcloud.cpp

TRANSLATIONS += languages/pluginlvoxv2_en.ts \
                languages/pluginlvoxv2_fr.ts

INCLUDEPATH += .
INCLUDEPATH += ./step
