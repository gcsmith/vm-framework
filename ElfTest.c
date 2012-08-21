/*
** File:	ElfTest.c
**
** Author:	Garrett Smith
**
** Contributor:
**
** Description:	Inspect the various ELF file and program headers.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "elf.h"

const char *UsageStr = "usage: ElfTest objectfile";

int main(int argc, char *argv[]) {
    int i;
    FILE *fp;
    Elf32_Ehdr header;
    unsigned char *buffer;

    if (argc != 2) {
        printf("%s\n%s\n", UsageStr, "invalid number of arguments specified");
        return 1;
    }

    fp = fopen(argv[1], "r");
    fread((void *)&header, 1, sizeof(Elf32_Ehdr), fp);

    printf("sizeof(Elf32_Ehdr) = %zu\n", sizeof(Elf32_Ehdr));
    printf("sizeof(Elf32_Phdr) = %zu\n", sizeof(Elf32_Phdr));
    printf("sizeof(Elf32_Shdr) = %zu\n\n", sizeof(Elf32_Shdr));

    printf("[ ELF File Header ]\n\n");
    printf("type     0x%X\n", (int)header.e_type);
    printf("machine  0x%X\n", (int)header.e_machine);
    printf("version  0x%X\n", (int)header.e_version);
    printf("entry    0x%X\n", (int)header.e_entry);
    printf("phoff    0x%X\n", (int)header.e_phoff);
    printf("shoff    0x%X\n", (int)header.e_shoff);
    printf("flags    0x%X\n", (int)header.e_flags);
    printf("ehsize   0x%X\n", (int)header.e_ehsize);
    printf("phentsz  0x%X\n", (int)header.e_phentsize);
    printf("phnnum   0x%X\n", (int)header.e_phnum);
    printf("shentsz  0x%X\n", (int)header.e_shentsize);
    printf("shnum    0x%X\n", (int)header.e_shnum);
    printf("shstrndx 0x%X\n\n", (int)header.e_shstrndx);

    /* iterate over process headers */
    fseek(fp, header.e_phoff, SEEK_SET);

    buffer = (unsigned char *)malloc(header.e_phentsize);

    for (i = 0; i < header.e_phnum; i++) {
        /* read in the program header */
        Elf32_Phdr *pheader = (Elf32_Phdr *)buffer;
        fread((void *)buffer, 1, header.e_phentsize, fp);

        printf("[ Program Header %d ]\n\n", i);
        printf("type   0x%X\n", (int)pheader->p_type);
        printf("offset 0x%X\n", (int)pheader->p_offset);
        printf("vaddr  0x%X\n", (int)pheader->p_vaddr);
        printf("paddr  0x%X\n", (int)pheader->p_paddr);
        printf("filesz 0x%X\n", (int)pheader->p_filesz);
        printf("memsz  0x%X\n", (int)pheader->p_memsz);
        printf("flags  0x%X\n", (int)pheader->p_flags);
        printf("align  0x%X\n\n", (int)pheader->p_align);
    }

    fclose(fp);
    return 0;
}

