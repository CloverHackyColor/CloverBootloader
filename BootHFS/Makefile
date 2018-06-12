BOOTSECTORS = boot0af boot0ss boot0md boot1h boot1h2 boot1f32 boot1f32alt boot1x boot1xalt
TEXTFILES   = Description.txt Installation.txt

BOOTSECTOR_SRCS = $(addsuffix .s, $(BOOTSECTORS))
BOOTSECTOR_BINS = $(addprefix $(DESTDIR)/, $(BOOTSECTORS))

TEXTFILES_TARGET = $(addprefix $(DESTDIR)/, $(TEXTFILES))

ifeq ($(DESTDIR),)
$(error Variable DESTDIR must be defined !)
endif

ifdef NASM_PREFIX
NASM=$(NASM_PREFIX)nasm
else
NASM=nasm
endif

all: $(BOOTSECTOR_BINS) $(PBOOTSECTOR_BINS) $(TEXTFILES_TARGET)

$(BOOTSECTOR_BINS): boot0.s $(BOOTSECTOR_SRCS)
	@echo "[NASM] $(@F).s -> $@"
	@"$(NASM)" $(@F).s -o $@

$(TEXTFILES_TARGET): $(TEXTFILES)
	@cp -v $(@F) $@

clean:
	rm -f $(BOOTSECTOR_BINS) $(PBOOTSECTOR_BINS) $(TEXTFILES_TARGET) *~
# Dependencies for File: ú1ÀŽÐ¼ðÿûŽØŽÀV¾
