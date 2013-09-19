#!/bin/sh
git clone git://git.seabios.org/seabios.git seabios
patch -p0 < seabiospatches.diff
