include(../../pluginsharedv2/shared.pri)
include($${PLUGIN_SHARED_DIR}/include.pri)

TARGET = plug_lvoxv2

HEADERS += $${PLUGIN_SHARED_INTERFACE_DIR}/interfaces.h \
    lvox_steppluginmanager.h \
    lvox_pluginentry.h \
    gridfactory.h \
    algorithmewoo.h \
    abstractvisitorraytracing.h \
    visitorraytracingincrement.h \
    visitorraytracingadddistance.h \
    computegridtools.h \
    visitorraytracingincrementbutfirst.h \
    visitorraytracingadddistancebutfirst.h \
    mymaths.h \
#    step/stepcomputedensityonly.h \
#    step/steploadlvoxinputfiles.h \
#    step/stepcomputedensityonlymultiscan.h \
#    step/stepcomputedensityanddistancesmultiscan.h \
    step/steploadfileldf.h \
    step/steploadfileasc.h \
#    step/steploadfilevox.h \
#    step/steploadfilemdf.h \
#    step/steploadfilemox.h \
#    step/stepcomputevisibility.h \
    step/stepcomputedensityanddistances.h

SOURCES += \
    lvox_pluginentry.cpp \
    lvox_steppluginmanager.cpp \
    gridfactory.cpp \
    algorithmewoo.cpp \
    abstractvisitorraytracing.cpp \
    visitorraytracingincrement.cpp \
    visitorraytracingadddistance.cpp \
    computegridtools.cpp \
    visitorraytracingincrementbutfirst.cpp \
    visitorraytracingadddistancebutfirst.cpp \  
    mymaths.cpp \
#    step/steploadlvoxinputfiles.cpp \
#    step/stepcomputedensityonly.cpp \
#    step/stepcomputedensityonlymultiscan.cpp \
#    step/stepcomputedensityanddistancesmultiscan.cpp \
    step/steploadfileldf.cpp \
    step/steploadfileasc.cpp \
#    step/steploadfilevox.cpp \
#    step/steploadfilemdf.cpp \
#    step/steploadfilemox.cpp \
#    step/stepcomputevisibility.cpp \
    step/stepcomputedensityanddistances.cpp

TRANSLATIONS    = plug_lvoxv2_en.ts

INCLUDEPATH += .
INCLUDEPATH += ./step
