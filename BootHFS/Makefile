BOOTSECTORS = boot0 boot0hfs boot0ab boot1h boot1altV3 boot1f32 boot1f32alt

all: $(BOOTSECTORS)

$(BOOTSECTORS):
	@echo "[NASM] $(@F)" 
	@nasm $(@F).s -o $@

install: $(BOOTSECTORS)
	@[ -n "$(DESTDIR)" ] || { echo "DESTDIR not defined" >&2;  exit 1; }
	@for p in $(BOOTSECTORS); do if [ -f "$${p}" ];then cp -v $${p} $(DESTDIR)/; fi; done

clean:
	@for p in $(BOOTSECTORS); do if [ -f "$${p}" ];then echo "[RM] $${p}"; rm -f "$${p}"; fi; done
