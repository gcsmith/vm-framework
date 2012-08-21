/*
** File:    elf.h
**
** Author:  Garrett Smith
**
** Contributor:
**
** Description: Types and definitions for the ELF executable file format.
**              Based off of the System V Specification v4.1
*/

#ifndef _ELF_H
#define _ELF_H

typedef uint32_t Elf32_Addr;
typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Off;
typedef uint32_t Elf32_Word;
typedef int32_t  Elf32_Sword;

/* e_indent[] Identification Indexes */

#define EI_MAG0     0   /* File identification */
#define EI_MAG1     1   /* File identification */
#define EI_MAG2     2   /* File identification */
#define EI_MAG3     3   /* File identification */
#define EI_CLASS    4   /* File class */
#define EI_DATA     5   /* Data encoding */
#define EI_VERSION  6   /* File version */
#define EI_PAD      7   /* Start of padding bytes */
#define EI_NIDENT   16  /* Size of e_ident[] */

/* values of e_indent[EI_MAG0] through e_indent[EI_MAG3] */

#define ELFMAG0     0x7f    /* e_ident[EI_MAG0] */
#define ELFMAG1     'E'     /* e_ident[EI_MAG1] */
#define ELFMAG2     'L'     /* e_ident[EI_MAG2] */
#define ELFMAG3     'F'     /* e_ident[EI_MAG3] */

/* values of e_indent[EI_CLASS] */

#define ELFCLASSNONE    0   /* Invalid class */
#define ELFCLASS32      1   /* 32-bit objects */
#define ELFCLASS64      2   /* 64-bit objects */

/* values of e_indnet[EI_DATA] */

#define ELFDATANONE     0   /* Invalid data encoding */
#define ELFDATA2LSB     1   /* See below */
#define ELFDATA2MSB     2   /* See below */

/* values of e_type
 * identifies the object file type */

#define ET_NONE     0       /* No file type */
#define ET_REL      1       /* Relocatable file */
#define ET_EXEC     2       /* Executable file */
#define ET_DYN      3       /* Shared object file */
#define ET_CORE     4       /* Core file */
#define ET_LOPROC   0xff00  /* Processor-specific */
#define ET_HIPROC   0xffff  /* Processor-specific */

/* values of e_machine 
 * specifies the required architecture for an individual file */

#define EM_M32          1   /* AT&T WE 32100 */
#define EM_SPARC        2   /* SPARC */
#define EM_386          3   /* Intel Architecture */
#define EM_68K          4   /* Motorola 68000 */
#define EM_88K          5   /* Motorola 88000 */
#define EM_860          7   /* Intel 80860 */
#define EM_MIPS         8   /* MIPS RS3000 Big-Endian */
#define EM_MIPS_RS4_BE  10  /* MIPS RS4000 Big-Endian */

/* values of e_version
 * identifies the object file version */

#define EV_NONE     0   /* Invalid version */
#define EV_CURRENT  1   /* Current version */

/* ELF Header */

typedef struct {
    unsigned char   e_ident[EI_NIDENT]; /* machine-independent identification */
    Elf32_Half      e_type;             /* object file type */
    Elf32_Half      e_machine;          /* required architecture */
    Elf32_Word      e_version;          /* object file version */
    Elf32_Addr      e_entry;            /* virtual address of entrypoint */
    Elf32_Off       e_phoff;            /* program header table offset */
    Elf32_Off       e_shoff;            /* section header table offset */
    Elf32_Word      e_flags;            /* processor-specific flags */
    Elf32_Half      e_ehsize;           /* ELF header size */
    Elf32_Half      e_phentsize;        /* program header table entry size */
    Elf32_Half      e_phnum;            /* program header table entry count */
    Elf32_Half      e_shentsize;        /* section header table entry size */
    Elf32_Half      e_shnum;            /* section header table entry count */
    Elf32_Half      e_shstrndx;         /* string table section index */
} Elf32_Ehdr;

/* special section indexes */

#define SHN_UNDEF       0
#define SHN_LORESERVE   0xff00
#define SHN_LOPROC      0xff00
#define SHN_HIPROC      0xff1f
#define SHN_ABS         0xfff1
#define SHN_COMMON      0xfff2
#define SHN_HIRESERVE   0xffff

/* values of sh_type
 * specifies the section type */

#define SHT_NULL        0
#define SHT_PROGBITS    1
#define SHT_SYMTAB      2
#define SHT_STRTAB      3
#define SHT_RELA        4
#define SHT_HASH        5
#define SHT_DYNAMIC     6
#define SHT_NOTE        7
#define SHT_NOBITS      8
#define SHT_REL         9
#define SHT_SHLIB       10
#define SHT_DYNSYM      11
#define SHT_LOPROC      0x70000000
#define SHT_HIPROC      0x7fffffff
#define SHT_LOUSER      0x80000000
#define SHT_HIUSER      0xffffffff

/* values of sh_flags
 * specifies the section attribute flags */

#define SHF_WRITE       0x1
#define SHF_ALLOC       0x2
#define SHF_EXECINSTR   0x4
#define SHF_MASKPROC    0xf0000000

/* Section Header */

typedef struct {
    Elf32_Word  sh_name;        /* section name / index into string table */
    Elf32_Word  sh_type;        /* section type */
    Elf32_Word  sh_flags;       /* section attribute flags */
    Elf32_Addr  sh_addr;        /* vritual address of section */
    Elf32_Off   sh_offset;      /* offset to section */
    Elf32_Word  sh_size;        /* section size */
    Elf32_Word  sh_link;        /* section header table index link */
    Elf32_Word  sh_info;        /* extra information */
    Elf32_Word  sh_addralign;   /* address alignment constraints */
    Elf32_Word  sh_entsize;     /* fixed-size entry size */
} Elf32_Shdr;

/* values of p_type
 * specifies the segment type */

#define PT_NULL     0           /* unused element */
#define PT_LOAD     1           /* loadable segment */
#define PT_DYNAMIC  2           /* dynamic linking information */
#define PT_INTERP   3           /* location and size of path name to invoke */
#define PT_NOTE     4           /* location and size of auxiliary information */
#define PT_SHLIB    5           /* reserved */
#define PT_PHDR     6           /* location and size of program header table */
#define PT_TLS      7           /* thread local storage */
#define PT_LOOS     0x60000000  /* range of OS-specific semantics */
#define PT_HIOS     0x6fffffff  /* ... */
#define PT_LOPROC   0x70000000  /* range of processor-specific semantics */
#define PT_HIPROC   0x7fffffff  /* ... */

/* values of p_flags
 * specifies the segment flag bits */

#define PF_X        0x1         /* Execute */
#define PF_W        0x2         /* Write */
#define PF_R        0x4         /* Read */
#define PF_MASKPROC 0xf0000000  /* Unspecified */

/* Program Header */

typedef struct {
    Elf32_Word  p_type;         /* segment type */
    Elf32_Off   p_offset;       /* offset to segment in file */
    Elf32_Addr  p_vaddr;        /* virtual address of segment */
    Elf32_Addr  p_paddr;        /* physical address of segment (unspecified) */
    Elf32_Word  p_filesz;       /* size of segment in file */
    Elf32_Word  p_memsz;        /* size of segment image in memory */
    Elf32_Word  p_flags;        /* flags relevant to the segment */
    Elf32_Word  p_align;        /* alignment of segment in memory */
} Elf32_Phdr;

int _elf_valid(Elf32_Ehdr *header);
int _elf_load(Elf32_Ehdr *header, void *dest, void *src);

#endif

