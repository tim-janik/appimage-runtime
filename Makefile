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

# == runtime.c ==
runtime.o: src/runtime.c $(wildcard src/*.c src/*.h) Makefile
	$(CC) $(INCLUDES) $(DEFS) $(OPTIMIZE) $(PKG_CFLAGS) $(CFLAGS) -c $< -o $@

# == runtime ==
runtime: runtime.o
	$(CC) $(OPTIMIZE) $(PKG_CFLAGS) $(CFLAGS) $(LDFLAGS) \
		$< -o $@ $(LIBS) $(UNZLIBS:%=-l:lib%.a)
	strip $@
all: runtime

# == runtime-zstd ==
runtime-zstd: runtime.o src/mocklibs.c
	$(CC) $(OPTIMIZE) $(PKG_CFLAGS) $(CFLAGS) $(LDFLAGS) \
		src/mocklibs.c -DWITH_ZSTD \
		$< -o $@ $(LIBS) $(UNZLIBS:%=-l:lib%.a) \
		$(LIBS) -l:libzstd.a
	strip $@
all: runtime-zstd

# == clean ==
clean:
	rm -f *.o runtime runtime-zstd

