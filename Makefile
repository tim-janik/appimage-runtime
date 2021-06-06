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
TARGETS		::=

# == runtime.c ==
runtime.o: src/runtime.c $(wildcard src/*.c src/*.h)
	$(CC) $(INCLUDES) $(DEFS) $(OPTIMIZE) $(PKG_CFLAGS) $(CFLAGS) -c $< -o $@

# == appimage-runtime ==
appimage-runtime: runtime.o
	$(CC) $(OPTIMIZE) $(PKG_CFLAGS) $(CFLAGS) $(LDFLAGS) \
		$< -o $@ $(LIBS) $(UNZLIBS:%=-l:lib%.a)
	strip $@
TARGETS += appimage-runtime

# == appimage-runtime-zstd ==
appimage-runtime-zstd: runtime.o src/mocklibs.c
	$(CC) $(OPTIMIZE) $(PKG_CFLAGS) $(CFLAGS) $(LDFLAGS) \
		src/mocklibs.c -DWITH_ZSTD \
		$< -o $@ $(LIBS) $(UNZLIBS:%=-l:lib%.a) \
		$(LIBS) -l:libzstd.a
	strip $@
TARGETS += appimage-runtime-zstd

# == clean ==
clean:
	rm -f *.o appimage-runtime appimage-runtime-zstd

# == all ==
all: $(TARGETS)

# == release ==
release: all
	@ git merge-base --is-ancestor HEAD origin/trunk || { echo "ERROR: HEAD not published in origin/trunk" ; false ; }
	@ rm -f .release && D=`date +%y.%m | sed 's/\.0/./g'` && \
		for n in `seq 0 99` ; do git rev-parse $$D.$$n >/dev/null 2>&1 || { echo $$D.$$n >.release; break ; } ; done
	@ R=`cat .release` && rm -f .release && echo -e "\n# RELEASE: appimage-runtime v$$R:" && \
		echo "hub release create -dpo -m 'appimage-runtime v$$R' $$R    $(TARGETS:%=-a %)" && \
		echo '^^^^^^^^^^^^^^^^^^ Enter to execute... ' && \
		read && \
		hub release create -dpo -m "appimage-runtime v$$R" $$R    $(TARGETS:%=-a %)

