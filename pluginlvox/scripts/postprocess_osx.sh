#!/bin/bash

THISDIR="`pwd`"
DESTDIR="`pwd`/../../ComputreeInstall"

cd "$THISDIR"

# Generate translations
lrelease pluginonfensamv2.pro
mkdir -p $DESTDIR/languages
cp languages/*.qm $DESTDIR/languages