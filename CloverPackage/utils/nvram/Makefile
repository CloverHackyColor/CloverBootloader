
PROGRAM = nvram2

SRCROOT := $(abspath $(CURDIR)/..)
SYMROOT := $(abspath $(CURDIR)/../../sym)
OBJROOT := $(SYMROOT)/build/$(PROGRAM)
INSTALL_DIR_NAME=utils
UTILSDIR= $(SYMROOT)/$(INSTALL_DIR_NAME)


SRC  := $(wildcard $(PROGRAM)/*.c)
PROG := $(addprefix $(UTILSDIR)/, $(PROGRAM))

all: $(PROG)

$(PROG): $(SRC)
	@echo "\t[CXX] $(PROGRAM)"
	@cc -o $(PROGRAM) $(PROGRAM).c -framework CoreFoundation -framework IOKit -Wall >/dev/null
	@mv $(PROGRAM) $(UTILSDIR)

install-local: $(PROG)
	@sudo install -d -g 0 -o 0 /usr/local/bin
	@sudo install -psv -g 0 -o 0 $(PROG) /usr/local/bin

clean-local:
	@rm -rf $(OBJROOT) *~
