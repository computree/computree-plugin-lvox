COMPUTREE += ctlibio

MUST_USE_OPENCV = 1

CT_PREFIX_INSTALL = ../../..
CT_PREFIX = ../../../computreev3

include(../../../computreev3/shared.pri)
include($${PLUGIN_SHARED_DIR}/include.pri)
include($${CT_PREFIX}/include_ct_library.pri)

# FIXME: use the include_all.pri, should not define manually this variable
# but required, otherwise the build fails with error: ‘CT_Image2D’ does not name a type
DEFINES += USE_OPENCV

INCLUDEPATH += ../../pluginlvox/
INCLUDEPATH += ../../pluginlvox/muParser/include

# rpath works only on Unix
QMAKE_RPATHDIR += $${PLUGINSHARED_DESTDIR}
QMAKE_RPATHDIR += $${PLUGINSHARED_DESTDIR}/plugins/

QT       += testlib

QT       -= gui

TARGET = tst_grid_mergetest
CONFIG   += console
CONFIG   -= app_bundle
CONFIG += c++11

TEMPLATE = app

SOURCES += tst_grid_mergetest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

LIBS += -L$${PLUGINSHARED_DESTDIR}/plugins/ -lplug_lvoxv2
