#!/bin/sh

# boot1f32-install.sh
#
# Created by mackerintel on 2/2/09.
# Copyright 2009 mackerintel. All rights reserved.

if [[ x$1 == x ]]; then
	echo Usage: $0 disknumber;
	exit 0;
fi

if [[ `dd if=/dev/disk${1}s1 count=8 bs=1 skip=82 | uuencode -m -|head -n 2|tail -n 1` != "RkFUMzIgICA=" ]]; then
	echo "/dev/disk${1}s1" "isn't" a FAT32 partition;
	exit 1;
fi 

if [ ! -f boot1f32 ]; then
	echo "boot1f32 not found";
	exit 1;
fi

dd if=/dev/disk${1}s1 count=1 bs=512 of=/tmp/origbs
cp boot1f32 /tmp/newbs
dd if=/tmp/origbs of=/tmp/newbs skip=3 seek=3 bs=1 count=87 conv=notrunc
dd of=/dev/disk${1}s1 count=1 bs=512 if=/tmp/newbs
