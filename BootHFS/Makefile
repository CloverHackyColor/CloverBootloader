BOOTSECTORS = boot0 boot0hfs boot0ab boot1h boot1h2 boot1f32 boot1f32alt

BOOTSECTOR_BINS = $(addprefix $(DESTDIR)/, $(BOOTSECTORS))
BOOTSECTOR_SRCS = $(addsuffix .s, $(BOOTSECTORS))

ifeq ($(DESTDIR),)
$(error Variable DESTDIR must be defined !)
endif

all: $(BOOTSECTOR_BINS)

$(BOOTSECTOR_BINS): $(BOOTSECTOR_SRCS)
	@echo "[NASM] $(@F).s -> $@" 
	@nasm $(@F).s -o $@
