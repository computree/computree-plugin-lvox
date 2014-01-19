include(../../pluginsharedv2/shared.pri)
include($${PLUGIN_SHARED_DIR}/include.pri)

contains ( QT_VERSION, "^5.*" ) {
    LIBS += -lQt5Concurrent
}

TARGET = plug_lvoxv2

HEADERS += $${PLUGIN_SHARED_INTERFACE_DIR}/interfaces.h \
    lvox_steppluginmanager.h \
    lvox_pluginentry.h \
#    step/stepcomputedensityonly.h \
#    step/steploadlvoxinputfiles.h \
#    step/stepcomputedensityonlymultiscan.h \
#    step/stepcomputedensityanddistancesmultiscan.h \
#    step/steploadfilevox.h \
#    step/steploadfilemdf.h \
#    step/steploadfilemox.h \
#    step/stepcomputevisibility.h \
    tools/lvox_distancevisitor.h \
    tools/lvox_countvisitor.h \
    tools/lvox_computehitsthread.h \
    tools/lvox_computetheoriticalsthread.h \
    tools/lvox_computebeforethread.h \
    step/lvox_stepcomputelvoxgrids.h \
    tools/lvox_computedensitythread.h

SOURCES += \
    lvox_pluginentry.cpp \
    lvox_steppluginmanager.cpp \
#    step/steploadlvoxinputfiles.cpp \
#    step/stepcomputedensityonly.cpp \
#    step/stepcomputedensityonlymultiscan.cpp \
#    step/stepcomputedensityanddistancesmultiscan.cpp \
#    step/steploadfilevox.cpp \
#    step/steploadfilemdf.cpp \
#    step/steploadfilemox.cpp \
#    step/stepcomputevisibility.cpp \
    tools/lvox_distancevisitor.cpp \
    tools/lvox_countvisitor.cpp \
    tools/lvox_computehitsthread.cpp \
    tools/lvox_computetheoriticalsthread.cpp \
    tools/lvox_computebeforethread.cpp \
    step/lvox_stepcomputelvoxgrids.cpp \
    tools/lvox_computedensitythread.cpp

TRANSLATIONS    = plug_lvoxv2_en.ts

INCLUDEPATH += .
INCLUDEPATH += ./step
