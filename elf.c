/*
** File:	elf.c
**
** Author:	Garrett Smith
**
** Contributor:
**
** Description:	ELF format helper functions.
*/

#include "types.h"
#include "elf.h"
#include "klib.h"
#include "c_io.h"

int _elf_valid(Elf32_Ehdr *header) {
    if ((ELFMAG0 != header->e_ident[EI_MAG0]) ||
        (ELFMAG1 != header->e_ident[EI_MAG1]) ||
        (ELFMAG2 != header->e_ident[EI_MAG2]) ||
        (ELFMAG3 != header->e_ident[EI_MAG3])) {
        c_puts("_elf_valid: expected properly formatted elf image\n");
        return 0;
    }

    if (ELFCLASS32 != header->e_ident[EI_CLASS]) {
        c_puts("_elf_valid: expected 32-bit class elf image\n");
        return 0;
    }

    if (ELFDATA2LSB != header->e_ident[EI_DATA]) {
        c_puts("_elf_valid: expected little-endian encoding\n");
        return 0;
    }

    if ((EV_CURRENT != header->e_ident[EI_VERSION]) ||
        (EV_CURRENT != header->e_version)) {
        c_puts("_elf_valid: unexpected version number\n");
        return 0;
    }

    if (ET_EXEC != header->e_type) {
        c_puts("_elf_valid: expected executable image type\n");
        return 0;
    }

    if (EM_386 != header->e_machine) {
        c_puts("_elf_valid: expected i386 instruction format\n");
        return 0;
    }

    return 1;
}

int _elf_load(Elf32_Ehdr *header, void *dst, void *src) {
    /* iterate over the program headers */
    Elf32_Phdr *pheader = (Elf32_Phdr *)(src + header->e_phoff);
    int i;

    for (i = 0; i < header->e_phnum; i++) {
        /* only copy over the loadable segments. no user programs should
         * be linked with dynamic or tls segments, but its possible that the
         * gnu tools might slip in a PT_**OS type segment
         */
        if (pheader->p_type != PT_LOAD) {
            continue;
        }

        _memcpy((void *)(dst + pheader->p_vaddr),  /* source */
                (void *)(src + pheader->p_offset), /* destination */
                pheader->p_filesz);                /* size */

        ++pheader;
    }

    return 1;
}

