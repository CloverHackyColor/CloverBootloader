BOOTSECTORS = boot0af boot0ss boot0ab boot0md boot0xg boot1h boot1h2 boot1f32 boot1f32alt boot1x
TEXTFILES   = Description.txt Installation.txt

BOOTSECTOR_BINS = $(addprefix $(DESTDIR)/, $(BOOTSECTORS))
BOOTSECTOR_SRCS = $(addsuffix .s, $(BOOTSECTORS))

TEXTFILES_TARGET = $(addprefix $(DESTDIR)/, $(TEXTFILES))

ifeq ($(DESTDIR),)
$(error Variable DESTDIR must be defined !)
endif

all: $(BOOTSECTOR_BINS) $(TEXTFILES_TARGET)

$(BOOTSECTOR_BINS): $(BOOTSECTOR_SRCS)
	@echo "[NASM] $(@F).s -> $@" 
	@nasm $(@F).s -o $@

$(TEXTFILES_TARGET): $(TEXTFILES)
	@cp -v $(@F) $@

clean:
	rm -f $(BOOTSECTOR_BINS) $(TEXTFILES_TARGET)
