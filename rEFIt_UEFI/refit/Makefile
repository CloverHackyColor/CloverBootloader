#
# refit/Makefile
# Build control file for the rEFIt boot menu
#

LOCAL_CPPFLAGS  = -I. -I../include -I../libeg
LOCAL_LDFLAGS   = -L../libeg
LOCAL_LIBS      = -leg

OBJS            = main.o config.o menu.o screen.o icns.o lib.o
TARGET          = refit.efi

all: $(TARGET)

include ../Make.common

# EOF
