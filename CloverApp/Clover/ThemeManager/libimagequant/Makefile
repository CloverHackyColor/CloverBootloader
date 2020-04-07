-include config.mk

STATICLIB=libimagequant.a
SHAREDLIB=libimagequant.$(SOLIBSUFFIX)
SOVER=0
ifeq ($(SOLIBSUFFIX),dylib)
	SHAREDLIBVER=libimagequant.$(SOVER).$(SOLIBSUFFIX)
	FIX_INSTALL_NAME=install_name_tool -id $(LIBDIR)/$(SHAREDLIBVER) $(DESTDIR)$(LIBDIR)/$(SHAREDLIBVER)
else
	SHAREDLIBVER=libimagequant.$(SOLIBSUFFIX).$(SOVER)
	FIX_INSTALL_NAME=
endif

JNILIB=libimagequant.jnilib
DLL=imagequant.dll
DLLIMP=imagequant_dll.a
DLLDEF=imagequant_dll.def
JNIDLL=libimagequant.dll
JNIDLLIMP=libimagequant_dll.a
JNIDLLDEF=libimagequant_dll.def

OBJS = pam.o mediancut.o blur.o mempool.o kmeans.o nearest.o libimagequant.o
SHAREDOBJS = $(subst .o,.lo,$(OBJS))

JAVACLASSES = org/pngquant/LiqObject.class org/pngquant/PngQuant.class org/pngquant/Image.class org/pngquant/Result.class
JAVAHEADERS = $(JAVACLASSES:.class=.h)
JAVAINCLUDE = -I'$(JAVA_HOME)/include' -I'$(JAVA_HOME)/include/linux' -I'$(JAVA_HOME)/include/win32' -I'$(JAVA_HOME)/include/darwin'

DISTFILES = $(OBJS:.o=.c) *.h README.md CHANGELOG COPYRIGHT Makefile configure imagequant.pc.in
TARNAME = libimagequant-$(VERSION)
TARFILE = $(TARNAME)-src.tar.bz2
PKGCONFIG = imagequant.pc

all: static shared

static: $(STATICLIB)

shared: $(SHAREDLIB)

dll:
	$(MAKE) CFLAGS="$(CFLAGS) -DIMAGEQUANT_EXPORTS" $(DLL)

java: $(JNILIB)

java-dll:
	$(MAKE) CFLAGS="$(CFLAGS) -DIMAGEQUANT_EXPORTS" $(JNIDLL)

$(DLL) $(DLLIMP): $(OBJS)
	$(CC) -fPIC -shared -o $(DLL) $^ $(LDFLAGS) -Wl,--out-implib,$(DLLIMP),--output-def,$(DLLDEF)

$(STATICLIB): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

$(SHAREDOBJS):
	$(CC) -fPIC $(CFLAGS) -c $(@:.lo=.c) -o $@

libimagequant.so: $(SHAREDOBJS)
	$(CC) -shared -Wl,-soname,$(SHAREDLIBVER) -o $(SHAREDLIBVER) $^ $(LDFLAGS)
	ln -fs $(SHAREDLIBVER) $(SHAREDLIB)

libimagequant.dylib: $(SHAREDOBJS)
	$(CC) -shared -o $(SHAREDLIBVER) $^ $(LDFLAGS)
	ln -fs $(SHAREDLIBVER) $(SHAREDLIB)

$(OBJS): $(wildcard *.h) config.mk

$(JNILIB): $(JAVAHEADERS) $(STATICLIB) org/pngquant/PngQuant.c
	$(CC) -g $(CFLAGS) $(LDFLAGS) $(JAVAINCLUDE) -shared -o $@ org/pngquant/PngQuant.c $(STATICLIB)

$(JNIDLL) $(JNIDLLIMP): $(JAVAHEADERS) $(OBJS) org/pngquant/PngQuant.c
	$(CC) -fPIC -shared -I. $(JAVAINCLUDE) -o $(JNIDLL) $^ $(LDFLAGS) -Wl,--out-implib,$(JNIDLLIMP),--output-def,$(JNIDLLDEF)

$(JAVACLASSES): %.class: %.java
	javac $<

$(JAVAHEADERS): %.h: %.class
	javah -o $@ $(subst /,., $(patsubst %.class,%,$<)) && touch $@

dist: $(TARFILE) cargo

$(TARFILE): $(DISTFILES)
	rm -rf $(TARFILE) $(TARNAME)
	mkdir $(TARNAME)
	cp $(DISTFILES) $(TARNAME)
	tar -cjf $(TARFILE) --numeric-owner --exclude='._*' $(TARNAME)
	rm -rf $(TARNAME)
	-shasum $(TARFILE)

cargo:
	rm -rf msvc-dist
	git clone . -b msvc msvc-dist
	rm -rf msvc-dist/Cargo.toml msvc-dist/org msvc-dist/rust msvc-dist/README.md msvc-dist/COPYRIGHT
	cargo test

example: example.c lodepng.h lodepng.c $(STATICLIB)
	$(CC) -g $(CFLAGS) -Wall example.c $(STATICLIB) -o example

lodepng.h:
	curl -o lodepng.h -L https://raw.githubusercontent.com/lvandeve/lodepng/master/lodepng.h

lodepng.c:
	curl -o lodepng.c -L https://raw.githubusercontent.com/lvandeve/lodepng/master/lodepng.cpp

clean:
	rm -f $(OBJS) $(SHAREDOBJS) $(SHAREDLIBVER) $(SHAREDLIB) $(STATICLIB) $(TARFILE) $(DLL) '$(DLLIMP)' '$(DLLDEF)'
	rm -f $(JAVAHEADERS) $(JAVACLASSES) $(JNILIB) example

distclean: clean
	rm -f config.mk
	rm -f imagequant.pc

install: all $(PKGCONFIG)
	install -d $(DESTDIR)$(LIBDIR)
	install -d $(DESTDIR)$(PKGCONFIGDIR)
	install -d $(DESTDIR)$(INCLUDEDIR)
	install -m 644 $(STATICLIB) $(DESTDIR)$(LIBDIR)/$(STATICLIB)
	install -m 644 $(SHAREDLIBVER) $(DESTDIR)$(LIBDIR)/$(SHAREDLIBVER)
	ln -sf $(SHAREDLIBVER) $(DESTDIR)$(LIBDIR)/$(SHAREDLIB)
	install -m 644 $(PKGCONFIG) $(DESTDIR)$(PKGCONFIGDIR)/$(PKGCONFIG)
	install -m 644 libimagequant.h $(DESTDIR)$(INCLUDEDIR)/libimagequant.h
	$(FIX_INSTALL_NAME)

uninstall:
	rm -f $(DESTDIR)$(LIBDIR)/$(STATICLIB)
	rm -f $(DESTDIR)$(LIBDIR)/$(SHAREDLIBVER)
	rm -f $(DESTDIR)$(LIBDIR)/$(SHAREDLIB)
	rm -f $(DESTDIR)$(PKGCONFIGDIR)/$(PKGCONFIG)
	rm -f $(DESTDIR)$(INCLUDEDIR)/libimagequant.h

config.mk:
ifeq ($(filter %clean %distclean, $(MAKECMDGOALS)), )
	./configure
endif

$(PKGCONFIG): config.mk
	sed 's|PREFIX|$(PREFIX)|;s|VERSION|$(VERSION)|' < imagequant.pc.in > $(PKGCONFIG)

.PHONY: all static shared clean dist distclean dll java cargo
.DELETE_ON_ERROR:
