#!/bin/sh
TOOLCHAIN=`echo $(cd ../../../CloverTools/bin/; pwd)`
PREFIX=x86_64-linux-gnu

cd seabios

make $1 \
  HOSTCC=gcc \
  CC=$TOOLCHAIN/$PREFIX-gcc \
  LD=$TOOLCHAIN/$PREFIX-ld \
  CPP=$TOOLCHAIN/$PREFIX-cpp \
  OBJCOPY=$TOOLCHAIN/$PREFIX-objcopy \
  OBJDUMP=$TOOLCHAIN/$PREFIX-objdump \
  STRIP=$TOOLCHAIN/$PREFIX-strip \
  #IASL=/usr/local/bin/iasl

cd ..
