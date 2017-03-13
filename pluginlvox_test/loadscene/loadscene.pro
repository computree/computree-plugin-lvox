#-------------------------------------------------
#
# Project created by QtCreator 2017-03-09T09:11:43
#
#-------------------------------------------------

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

# rpath works only on Unix
QMAKE_RPATHDIR += $${PLUGINSHARED_DESTDIR}
QMAKE_RPATHDIR += $${PLUGINSHARED_DESTDIR}/plugins/

QT       += testlib

QT       -= gui

TARGET = tst_loadscenetest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += tst_loadscenetest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

LIBS += -L$${PLUGINSHARED_DESTDIR}/plugins/ -lplug_lvoxv2

DISTFILES += \
    conf.in \
    conf.json \
    bad_file.in

HEADERS +=

# make sure changes to headers forces rebuild of the test
DEPENDPATH += $${INCLUDEPATH}
