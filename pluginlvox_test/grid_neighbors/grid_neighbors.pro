#-------------------------------------------------
#
# Project created by QtCreator 2017-02-07T15:10:56
#
#-------------------------------------------------
CT_PREFIX_INSTALL = ../../..
CT_PREFIX = ../../../computreev3

include(../../../computreev3/shared.pri)
include($${PLUGIN_SHARED_DIR}/include.pri)

# FIXME: use the include_all.pri, should not define manually this variable
# but required, otherwise the build fails with error: ‘CT_Image2D’ does not name a type
DEFINES += USE_OPENCV

INCLUDEPATH += ../../pluginlvox/

QMAKE_RPATHDIR += $${PLUGINSHARED_DESTDIR}

QT       += testlib

QT       -= gui

TARGET = tst_grid_neighborstest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += tst_grid_neighborstest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
