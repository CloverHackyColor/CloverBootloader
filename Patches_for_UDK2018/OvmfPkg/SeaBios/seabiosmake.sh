#!/bin/sh
TOOLCHAIN=`echo $(cd ../../../toolchain/cross/bin/; pwd)`
PREFIX=x86_64-clover-linux-gnu

cd seabios

make $1 \
  HOSTCC=gcc \
  CC=$TOOLCHAIN/$PREFIX-gcc \
  LD=$TOOLCHAIN/$PREFIX-ld \
  CPP=$TOOLCHAIN/$PREFIX-cpp \
  OBJCOPY=$TOOLCHAIN/$PREFIX-objcopy \
  OBJDUMP=$TOOLCHAIN/$PREFIX-objdump \
  STRIP=$TOOLCHAIN/$PREFIX-strip \
  AS=$TOOLCHAIN/$PREFIX-as \
  #IASL=/usr/local/bin/iasl

cd ..
