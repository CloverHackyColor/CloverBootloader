BOOTSECTORS = boot0 boot0hfs boot0ab boot1h boot1h2 boot1f32 boot1f32alt

BOOTSECTOR_BINS = $(addprefix $(DESTDIR)/, $(BOOTSECTORS))
BOOTSECTOR_SRCS = $(addsuffix .s, $(BOOTSECTORS))


all: $(BOOTSECTOR_BINS)

$(BOOTSECTOR_BINS): $(BOOTSECTOR_SRCS)
	@echo "[NASM] $(@F).s -> $@" 
	@nasm $(@F).s -o $@

install: $(BOOTSECTOR_BINS)
	@[ -n "$(DESTDIR)" ] || { echo "DESTDIR not defined" >&2;  exit 1; }
