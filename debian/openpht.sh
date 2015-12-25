#!/bin/sh

PHTROOT=/usr/lib/openpht

XBMC_HOME=$PHTROOT/share/XBMC
export XBMC_HOME
#Use export AE_ENGINE=SOFT to disable pulse audio
#export AE_ENGINE=SOFT
$PHTROOT/bin/plexhometheater

