SHELL         ::= /bin/bash -o pipefail

all: # default target

CC		 ?= clang
CFLAGS		 ?=
INCLUDES	::= -I.
DEFS		::= -DGIT_COMMIT='"$(shell git describe)"'
PKG_CFLAGS	::= -Wall $(shell pkg-config --static --cflags squashfuse libsquashfs0)
OPTIMIZE	::= -Os -flto
LDFLAGS		::= -pthread
SQUASHLIBS	::= squashfs squashfuse squashfuse_ll fuse fuseprivate
UNZLIBS		::= zstd lz4 z lzo2 lzma
LIBS		::= -ldl $(SQUASHLIBS:%=-l:lib%.a)
MAGIC_LD	::= -Xlinker -T -Xlinker src/magic.ld
DEPS		::= src/magic.ld
OBJECTS		::=
TARGETS		::=

# == runtime.c ==
runtime.o: src/runtime.c $(wildcard src/*.c src/*.h)
	$(CC) $(INCLUDES) $(DEFS) $(OPTIMIZE) $(PKG_CFLAGS) $(CFLAGS) -c $< -o $@
OBJECTS += runtime.o

# == appimage-runtime ==
appimage-runtime: $(OBJECTS) $(DEPS)
	$(CC) $(OPTIMIZE) $(PKG_CFLAGS) $(CFLAGS) $(LDFLAGS) \
		$(OBJECTS) -o $@ $(LIBS) $(UNZLIBS:%=-l:lib%.a) $(MAGIC_LD)
	strip $@
TARGETS += appimage-runtime

# == appimage-runtime-zstd ==
appimage-runtime-zstd: $(OBJECTS) $(DEPS) src/mocklibs.c
	$(CC) $(OPTIMIZE) $(PKG_CFLAGS) $(CFLAGS) $(LDFLAGS) \
		src/mocklibs.c -DWITH_ZSTD \
		$(OBJECTS) -o $@ $(LIBS) $(UNZLIBS:%=-l:lib%.a) \
		-l:libzstd.a $(MAGIC_LD)
	strip $@
TARGETS += appimage-runtime-zstd

# == clean ==
clean:
	rm -f *.o appimage-runtime appimage-runtime-zstd

# == all ==
all: $(TARGETS)

# == release ==
release:
	@ git merge-base --is-ancestor HEAD origin/trunk || { echo "ERROR: HEAD not published in origin/trunk" ; false ; }
	@ git describe --tags --exact-match --match '[0-9]*.[0-9]*.[0-9]*' >/dev/null 2>&1 || { \
	    D=`date +%y.%m | sed 's/\.0/./g'` && for n in `seq 0 99` ; do \
		R="$$D.$$n" && git rev-parse $$R >/dev/null 2>&1 || { \
		echo "ERROR: missing release tag - run:" && \
		echo "git tag -a $$R -m 'appimage-runtime v$$R' && make clean"; \
		exit 1; } ; done ; }
	@ R=`git describe --tags --exact-match --match '[0-9]*.[0-9]*.[0-9]*' ` && \
		rm -f .release && echo "# RELEASE: appimage-runtime v$$R:" && \
		echo "hub release create -dpo -m 'appimage-runtime v$$R' $$R    $(TARGETS:%=-a %)" && \
		read -p '^^^^^^^^^^^^^^^^^^ Enter to execute... ' && \
		hub release create -dpo -m "appimage-runtime v$$R" $$R    $(TARGETS:%=-a %)

