#
# libeg/libeg.mak
# Build control file for the libeg library
#

#
# Include sdk.env environment
#

!include $(SDK_INSTALL_DIR)\build\$(SDK_BUILD_ENV)\sdk.env

#
#  Set the base output name
#

BASE_NAME = libeg

#
# Globals needed by master.mak
#

TARGET_LIB = $(BASE_NAME)
SOURCE_DIR = $(SDK_INSTALL_DIR)\refit\$(BASE_NAME)
BUILD_DIR  = $(SDK_BUILD_DIR)\refit\$(BASE_NAME)

#
# Include paths
#

!include $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR)\makefile.hdr
INC = -I $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR) \
      -I $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR)\$(PROCESSOR)

#!include .\makefile.hdr
INC = -I . \
      -I .\$(PROCESSOR) $(INC)

#
# Default target
#

all : dirs $(OBJECTS)

#
#  Library object files
#

OBJECTS = $(OBJECTS) \
    $(BUILD_DIR)\screen.obj \
    $(BUILD_DIR)\image.obj \
    $(BUILD_DIR)\text.obj \
    $(BUILD_DIR)\load_bmp.obj \
    $(BUILD_DIR)\load_icns.obj \

#
# Source file dependencies
#

$(BUILD_DIR)\screen.obj           : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\image.obj            : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\text.obj             : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\load_bmp.obj         : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\load_icns.obj        : $(*B).c $(INC_DEPS)

#
# Handoff to master.mak
#

!include $(SDK_INSTALL_DIR)\build\master.mak
