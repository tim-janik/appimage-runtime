#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>
#include <memory.h>
#include <sys/mman.h>
#include <assert.h>

#include "light_elf.h"
#include "light_byteswap.h"


typedef Elf32_Nhdr Elf_Nhdr;

static char *fname;
static Elf64_Ehdr ehdr;

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define ELFDATANATIVE ELFDATA2LSB
#elif __BYTE_ORDER == __BIG_ENDIAN
#define ELFDATANATIVE ELFDATA2MSB
#else
#error "Unknown machine endian"
#endif

static uint16_t file16_to_cpu(uint16_t val)
{
	if (ehdr.e_ident[EI_DATA] != ELFDATANATIVE)
		val = bswap_16(val);
	return val;
}

static uint32_t file32_to_cpu(uint32_t val)
{
	if (ehdr.e_ident[EI_DATA] != ELFDATANATIVE)
		val = bswap_32(val);
	return val;
}

static uint64_t file64_to_cpu(uint64_t val)
{
	if (ehdr.e_ident[EI_DATA] != ELFDATANATIVE)
		val = bswap_64(val);
	return val;
}

static off_t read_elf32(FILE* fd)
{
	Elf32_Ehdr ehdr32;
	Elf32_Shdr shdr32;
	off_t last_shdr_offset;
	ssize_t ret;
    off_t  sht_end, last_section_end;

	fseeko(fd, 0, SEEK_SET);
	ret = fread(&ehdr32, 1, sizeof(ehdr32), fd);
	if (ret < 0 || (size_t)ret != sizeof(ehdr32)) {
		fprintf(stderr, "Read of ELF header from %s failed: %s\n",
			fname, strerror(errno));
		return -1;
	}

	ehdr.e_shoff		= file32_to_cpu(ehdr32.e_shoff);
	ehdr.e_shentsize	= file16_to_cpu(ehdr32.e_shentsize);
	ehdr.e_shnum		= file16_to_cpu(ehdr32.e_shnum);

	last_shdr_offset = ehdr.e_shoff + (ehdr.e_shentsize * (ehdr.e_shnum - 1));
    fseeko(fd, last_shdr_offset, SEEK_SET);
    ret = fread(&shdr32, 1, sizeof(shdr32), fd);
	if (ret < 0 || (size_t)ret != sizeof(shdr32)) {
		fprintf(stderr, "Read of ELF section header from %s failed: %s\n",
			fname, strerror(errno));
		return -1;
	}

	/* ELF ends either with the table of section headers (SHT) or with a section. */
	sht_end = ehdr.e_shoff + (ehdr.e_shentsize * ehdr.e_shnum);
	last_section_end = file64_to_cpu(shdr32.sh_offset) + file64_to_cpu(shdr32.sh_size);
	return sht_end > last_section_end ? sht_end : last_section_end;
}

static off_t read_elf64(FILE* fd)
{
	Elf64_Ehdr ehdr64;
	Elf64_Shdr shdr64;
	off_t last_shdr_offset;
	off_t ret;
    off_t sht_end, last_section_end;

	fseeko(fd, 0, SEEK_SET);
	ret = fread(&ehdr64, 1, sizeof(ehdr64), fd);
	if (ret < 0 || (size_t)ret != sizeof(ehdr64)) {
		fprintf(stderr, "Read of ELF header from %s failed: %s\n",
			fname, strerror(errno));
		return -1;
	}

	ehdr.e_shoff		= file64_to_cpu(ehdr64.e_shoff);
	ehdr.e_shentsize	= file16_to_cpu(ehdr64.e_shentsize);
	ehdr.e_shnum		= file16_to_cpu(ehdr64.e_shnum);

	last_shdr_offset = ehdr.e_shoff + (ehdr.e_shentsize * (ehdr.e_shnum - 1));
    fseeko(fd, last_shdr_offset, SEEK_SET);
    ret = fread(&shdr64, 1, sizeof(shdr64), fd);
	if (ret < 0 || ret != sizeof(shdr64)) {
		fprintf(stderr, "Read of ELF section header from %s failed: %s\n",
			fname, strerror(errno));
		return -1;
	}

	/* ELF ends either with the table of section headers (SHT) or with a section. */
	sht_end = ehdr.e_shoff + (ehdr.e_shentsize * ehdr.e_shnum);
	last_section_end = file64_to_cpu(shdr64.sh_offset) + file64_to_cpu(shdr64.sh_size);
	return sht_end > last_section_end ? sht_end : last_section_end;
}

ssize_t appimage_get_elf_size(const char* fname) {
    off_t ret;
    FILE* fd = NULL;
    off_t size = -1;

	fd = fopen(fname, "rb");
	if (fd == NULL) {
		fprintf(stderr, "Cannot open %s: %s\n",
			fname, strerror(errno));
		return -1;
	}
	ret = fread(ehdr.e_ident, 1, EI_NIDENT, fd);
	if (ret != EI_NIDENT) {
		fprintf(stderr, "Read of e_ident from %s failed: %s\n",
			fname, strerror(errno));
		return -1;
	}
	if ((ehdr.e_ident[EI_DATA] != ELFDATA2LSB) &&
		(ehdr.e_ident[EI_DATA] != ELFDATA2MSB)) {
		fprintf(stderr, "Unknown ELF data order %u\n",
			ehdr.e_ident[EI_DATA]);
		return -1;
	}
	if (ehdr.e_ident[EI_CLASS] == ELFCLASS32) {
		size = read_elf32(fd);
	} else if (ehdr.e_ident[EI_CLASS] == ELFCLASS64) {
		size = read_elf64(fd);
	} else {
		fprintf(stderr, "Unknown ELF class %u\n", ehdr.e_ident[EI_CLASS]);
		return -1;
	}

	fclose(fd);
	return size;
}

/* Return the offset, and the length of an ELF section with a given name in a given ELF file */
bool appimage_get_elf_section_offset_and_length(const char* fname, const char* section_name, unsigned long* offset, unsigned long* length) {
	uint8_t* data;
	int i;
	int fd = open(fname, O_RDONLY);
	size_t map_size = (size_t) lseek(fd, 0, SEEK_END);

	data = mmap(NULL, map_size, PROT_READ, MAP_SHARED, fd, 0);
	close(fd);

	// this trick works as both 32 and 64 bit ELF files start with the e_ident[EI_NINDENT] section
	unsigned char class = data[EI_CLASS];

	if (class == ELFCLASS32) {
		Elf32_Ehdr* elf;
		Elf32_Shdr* shdr;

		elf = (Elf32_Ehdr*) data;
		shdr = (Elf32_Shdr*) (data + ((Elf32_Ehdr*) elf)->e_shoff);

		char* strTab = (char*) (data + shdr[elf->e_shstrndx].sh_offset);
		for (i = 0; i < elf->e_shnum; i++) {
			if (strcmp(&strTab[shdr[i].sh_name], section_name) == 0) {
				*offset = shdr[i].sh_offset;
				*length = shdr[i].sh_size;
			}
		}
	} else if (class == ELFCLASS64) {
		Elf64_Ehdr* elf;
		Elf64_Shdr* shdr;

		elf = (Elf64_Ehdr*) data;
		shdr = (Elf64_Shdr*) (data + elf->e_shoff);

		char* strTab = (char*) (data + shdr[elf->e_shstrndx].sh_offset);
		for (i = 0; i < elf->e_shnum; i++) {
			if (strcmp(&strTab[shdr[i].sh_name], section_name) == 0) {
				*offset = shdr[i].sh_offset;
				*length = shdr[i].sh_size;
			}
		}
	} else {
		fprintf(stderr, "Platforms other than 32-bit/64-bit are currently not supported!");
		munmap(data, map_size);
		return false;
	}

	munmap(data, map_size);
	return true;
}

char* read_file_offset_length(const char* fname, unsigned long offset, unsigned long length) {
	FILE* f;
	if ((f = fopen(fname, "r")) == NULL) {
		return NULL;
	}

	fseek(f, offset, SEEK_SET);

	char* buffer = calloc(length + 1, sizeof(char));
	size_t n = fread(buffer, length, sizeof(char), f);
        assert (n > 0);

	fclose(f);

	return buffer;
}

int appimage_print_hex(char* fname, unsigned long offset, unsigned long length) {
	char* data;
	if ((data = read_file_offset_length(fname, offset, length)) == NULL) {
		return 1;
	}

	for (long long k = 0; k < length && data[k] != '\0'; k++) {
		printf("%x", data[k]);
	}

	free(data);

	printf("\n");

	return 0;
}

int appimage_print_binary(char* fname, unsigned long offset, unsigned long length) {
	char* data;
	if ((data = read_file_offset_length(fname, offset, length)) == NULL) {
		return 1;
	}

	printf("%s\n", data);

	free(data);

	return 0;
}

