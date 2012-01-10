#
# refit/refit.mak
# Build control file for the rEFIt boot menu
#

#
# Include sdk.env environment
#

!include $(SDK_INSTALL_DIR)\build\$(SDK_BUILD_ENV)\sdk.env

#
# Set the base output name and entry point
#

BASE_NAME         = refit
IMAGE_ENTRY_POINT = RefitMain

#
# Globals needed by master.mak
#

TARGET_APP = $(BASE_NAME)
SOURCE_DIR = $(SDK_INSTALL_DIR)\refit\$(BASE_NAME)
BUILD_DIR  = $(SDK_BUILD_DIR)\refit\$(BASE_NAME)

#
# Include paths
#

!include $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR)\makefile.hdr
INC = -I $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR) \
      -I $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR)\$(PROCESSOR) \
      -I $(SDK_INSTALL_DIR)\refit\include \
      -I $(SDK_INSTALL_DIR)\refit\libeg $(INC)

#
# Libraries
#

LIBS = $(LIBS) $(SDK_BUILD_DIR)\lib\libefi\libefi.lib \
               $(SDK_BUILD_DIR)\refit\libeg\libeg.lib

#
# Default target
#

all : dirs $(LIBS) $(OBJECTS)
	@echo Copying $(BASE_NAME).efi to current directory
	@copy $(SDK_BIN_DIR)\$(BASE_NAME).efi $(BASE_NAME)_$(SDK_BUILD_ENV).efi

#
# Program object files
#

OBJECTS = $(OBJECTS) \
    $(BUILD_DIR)\main.obj \
    $(BUILD_DIR)\config.obj  \
    $(BUILD_DIR)\menu.obj \
    $(BUILD_DIR)\screen.obj \
    $(BUILD_DIR)\icns.obj \
    $(BUILD_DIR)\lib.obj \

#
# Source file dependencies
#

$(BUILD_DIR)\main.obj       : $(*B).c $(INC_DEPS) lib.h
$(BUILD_DIR)\config.obj     : $(*B).c $(INC_DEPS) lib.h
$(BUILD_DIR)\menu.obj       : $(*B).c $(INC_DEPS) lib.h
$(BUILD_DIR)\screen.obj     : $(*B).c $(INC_DEPS) lib.h
$(BUILD_DIR)\icns.obj       : $(*B).c $(INC_DEPS) lib.h
$(BUILD_DIR)\lib.obj        : $(*B).c $(INC_DEPS) lib.h

#
# Handoff to master.mak
#

!include $(SDK_INSTALL_DIR)\build\master.mak
