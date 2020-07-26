#include "elfutils.h"
#include <string.h>
#include <stdio.h>

typedef struct _pair
{
    uint64_t num;
    const char* str;
}
pair_t;

static pair_t _at_pairs[] =
{
    { AT_NULL, "AT_NULL" },
    { AT_IGNORE, "AT_IGNORE" },
    { AT_EXECFD, "AT_EXECFD" },
    { AT_PHDR, "AT_PHDR" },
    { AT_PHENT, "AT_PHENT" },
    { AT_PHNUM, "AT_PHNUM" },
    { AT_PAGESZ, "AT_PAGESZ" },
    { AT_BASE, "AT_BASE" },
    { AT_FLAGS, "AT_FLAGS" },
    { AT_ENTRY, "AT_ENTRY" },
    { AT_NOTELF, "AT_NOTELF" },
    { AT_UID, "AT_UID" },
    { AT_EUID, "AT_EUID" },
    { AT_GID, "AT_GID" },
    { AT_EGID, "AT_EGID" },
    { AT_PLATFORM, "AT_PLATFORM" },
    { AT_HWCAP, "AT_HWCAP" },
    { AT_CLKTCK, "AT_CLKTCK" },
    { AT_FPUCW, "AT_FPUCW" },
    { AT_DCACHEBSIZE, "AT_DCACHEBSIZE" },
    { AT_ICACHEBSIZE, "AT_ICACHEBSIZE" },
    { AT_UCACHEBSIZE, "AT_UCACHEBSIZE" },
    { AT_IGNOREPPC, "AT_IGNOREPPC" },
    { AT_SECURE, "AT_SECURE" },
    { AT_BASE_PLATFORM, "AT_BASE_PLATFORM" },
    { AT_RANDOM, "AT_RANDOM" },
    { AT_HWCAP2, "AT_HWCAP2" },
    { AT_EXECFN, "AT_EXECFN" },
    { AT_SYSINFO, "AT_SYSINFO" },
    { AT_SYSINFO_EHDR, "AT_SYSINFO_EHDR" },
    { AT_L1I_CACHESHAPE, "AT_L1I_CACHESHAPE" },
    { AT_L1D_CACHESHAPE, "AT_L1D_CACHESHAPE" },
    { AT_L2_CACHESHAPE, "AT_L2_CACHESHAPE" },
    { AT_L3_CACHESHAPE, "AT_L3_CACHESHAPE" },
    { AT_L1I_CACHESIZE, "AT_L1I_CACHESIZE" },
    { AT_L1I_CACHEGEOMETRY, "AT_L1I_CACHEGEOMETRY" },
    { AT_L1D_CACHESIZE, "AT_L1D_CACHESIZE" },
    { AT_L1D_CACHEGEOMETRY, "AT_L1D_CACHEGEOMETRY" },
    { AT_L2_CACHESIZE, "AT_L2_CACHESIZE" },
    { AT_L2_CACHEGEOMETRY, "AT_L2_CACHEGEOMETRY" },
    { AT_L3_CACHESIZE, "AT_L3_CACHESIZE" },
    { AT_L3_CACHEGEOMETRY, "AT_L3_CACHEGEOMETRY" },
    { AT_MINSIGSTKSZ, "AT_MINSIGSTKSZ" },
};

static size_t _n_at_pairs = sizeof(_at_pairs) / sizeof(_at_pairs[0]);

const char* elf64_at_string(uint64_t value)
{
    for (size_t i = 0; i < _n_at_pairs; i++)
    {
        if (value == _at_pairs[i].num)
            return _at_pairs[i].str;
    }

    return NULL;
}

static pair_t _pt_pairs[] =
{
    { PT_NULL, "PT_NULL" },
    { PT_LOAD, "PT_LOAD" },
    { PT_DYNAMIC, "PT_DYNAMIC" },
    { PT_INTERP, "PT_INTERP" },
    { PT_NOTE, "PT_NOTE" },
    { PT_SHLIB, "PT_SHLIB" },
    { PT_PHDR, "PT_PHDR" },
    { PT_TLS, "PT_TLS" },
    { PT_NUM, "PT_NUM" },
    { PT_LOOS, "PT_LOOS" },
    { PT_GNU_EH_FRAME, "PT_GNU_EH_FRAME" },
    { PT_GNU_STACK, "PT_GNU_STACK" },
    { PT_GNU_RELRO, "PT_GNU_RELRO" },
    { PT_LOSUNW, "PT_LOSUNW" },
    { PT_SUNWBSS, "PT_SUNWBSS" },
    { PT_SUNWSTACK, "PT_SUNWSTACK" },
    { PT_HISUNW, "PT_HISUNW" },
    { PT_HIOS, "PT_HIOS" },
    { PT_LOPROC, "PT_LOPROC" },
    { PT_HIPROC, "PT_HIPROC" },
};

static size_t _n_pt_pairs = sizeof(_pt_pairs) / sizeof(_pt_pairs[0]);

const char* elf64_pt_string(uint64_t value)
{
    for (size_t i = 0; i < _n_pt_pairs; i++)
    {
        if (value == _pt_pairs[i].num)
            return _pt_pairs[i].str;
    }

    return NULL;
}

static uint64_t _round_up_to_multiple(uint64_t x, uint64_t m)
{
    return (x + m - 1) / m * m;
}

/*
**==============================================================================
** elf_init_stack()
**
** Format:
**
**     [ argc           ]         8
**     [ argv[0]        ]         8
**     [ ...            ]         8
**     [ argv[argc]     ]         8
**     [ envp[0]        ]         8
**     [ ...            ]         8
**     [ envp[envc]     ]         8
**     [ auxv[0]        ]         16
**     [ ...            ]         16
**     [ auxv[auxc]     ]         16
**     [ padding        ]         (padding to 16 byte boundary)
**     [ argv strings   ]         >= 0
**     [ envp strings   ]         >= 0
**     [ padding        ]         (padding to 8 byte boundary)
**     [ end marker     ]         8
**     ...
**     [ stack bottom   ]
**
**==============================================================================
*/
int elf_init_stack(
    int argc,
    const char* argv[],
    size_t envc,
    const char* envp[],
    size_t auxc,
    const Elf64_auxv_t* auxv,
    void* stack, /* 16-byte aligned data */
    size_t stack_size)
{
    int ret = -1;
    size_t argv_offset;
    size_t envp_offset;
    size_t auxv_offset;
    size_t argv_strings_offset;
    size_t envp_strings_offset;
    size_t end_marker_offset;
    size_t end = 0;
    uint8_t* base;

    if (argc <= 0 || !argv || !stack || !stack_size)
        goto done;

    if (auxv == NULL && argc != 0)
        goto done;

    if (envp == NULL && envc != 0)
        goto done;

    /* Calculate the offset of argv[] (skip over argc) */
    argv_offset = sizeof(uint64_t);

    /* calculate offset of envp[] (skip over argv[]) */
    {
        envp_offset = argv_offset;

        /* Skip over argv[] elements (including null terminator) */
        envp_offset += sizeof(const char*) * (argc + 1);
    }

    /* calculate offset of auxv[] (skip over envp[]) */
    {
        auxv_offset = envp_offset;

        /* Skip over envp[] elements (including null terminator) */
        auxv_offset += sizeof(const char*) * (envc + 1);
    }

    /* calculate the offset of the argv[] strings (skip over auxv[]) */
    {
        argv_strings_offset = auxv_offset;

        /* Skip over auxv[] elements (including terminating element) */
        argv_strings_offset += (sizeof(Elf64_auxv_t)) * (auxc + 1);

        /* Pad to 16-byte boundary */
        argv_strings_offset = _round_up_to_multiple(argv_strings_offset, 16);
    }

    /* calculate the offset of the envp[] strings */
    {
        envp_strings_offset = argv_strings_offset;

        for (int i = 0; i < argc; i++)
            envp_strings_offset += strlen(argv[i]) + 1;
    }

    /* calculate the offset of the end marker */
    {
        end_marker_offset = envp_strings_offset;

        for (size_t i = 0; i < envc; i++)
            end_marker_offset += strlen(envp[i]) + 1;

        end_marker_offset = _round_up_to_multiple(end_marker_offset, 8);
    }

    /* calculate the total size of the data */
    end = end_marker_offset + sizeof(uint64_t);

    if (end > stack_size)
        goto done;

    base = stack;

    /* Initialize argc */
    *((uint64_t*)base) = (uint64_t)argc;

    /* Initialize argv[] */
    {
        char** argv_out = (char**)(base + argv_offset);
        char* p = (char*)(base + argv_strings_offset);

        for (int i = 0; i < argc; i++)
        {
            size_t n = strlen(argv[i]) + 1;
            memcpy(p, argv[i], n);
            argv_out[i] = p;
            p += n;
        }

        /* Initialize the terminator */
        memset(&argv_out[argc], 0, sizeof(Elf64_auxv_t));
    }

    /* Initialize envp[] */
    {
        char** envp_out = (char**)(base + envp_offset);
        char* p = (char*)(base + envp_strings_offset);

        for (size_t i = 0; i < envc; i++)
        {
            size_t n = strlen(envp[i]) + 1;
            memcpy(p, envp[i], n);
            envp_out[i] = p;
            p += n;
        }

        envp_out[envc] = NULL;
    }

    /* Initialize auxv[] */
    {
        Elf64_auxv_t* auxv_out = (Elf64_auxv_t*)(base + auxv_offset);

        for (size_t i = 0; i < auxc; i++)
            auxv_out[i] = auxv[i];

        memset(&auxv_out[auxc], 0, sizeof(Elf64_auxv_t));
    }

    ret = 0;

done:

    return ret;
}

void elf_dump_stack(void* stack)
{
    int argc = (int)(*(uint64_t*)stack);
    char** argv = (char**)((uint8_t*)stack + sizeof(uint64_t));
    char** envp;
    int envc = 0;
    Elf64_auxv_t* auxv;

    printf("=== dump_stack(%lX)\n", (unsigned long)stack);

    printf("argc=%d\n", argc);

    for (int i = 0; i < argc; i++)
        printf("argv[%d]=%s\n", i, argv[i]);

    envp = (argv + argc + 1);

    for (int i = 0; envp[i]; i++)
    {
        printf("envp[%d]=%s\n", i, envp[i]);
        envc++;
    }

    auxv = (Elf64_auxv_t*)(envp + envc + 1);

    for (int i = 0; auxv[i].a_type; i++)
    {
        const Elf64_auxv_t a = auxv[i];
        printf("%s=%lX\n", elf64_at_string(a.a_type), a.a_un.a_val);
    }
}

int _test_header(const Elf64_Ehdr* ehdr)
{
    if (!ehdr)
        return -1;

    if (ehdr->e_ident[EI_MAG0] != 0x7f)
        return -1;

    if (ehdr->e_ident[EI_MAG1] != 'E')
        return -1;

    if (ehdr->e_ident[EI_MAG2] != 'L')
        return -1;

    if (ehdr->e_ident[EI_MAG3] != 'F')
        return -1;

    if (ehdr->e_ident[EI_CLASS] != ELFCLASS64)
        return -1;

    if (ehdr->e_ident[EI_DATA] != ELFDATA2LSB)
        return -1;

    if (ehdr->e_machine != EM_X86_64)
        return -1;

    if (ehdr->e_ehsize != sizeof(Elf64_Ehdr))
        return -1;

    if (ehdr->e_phentsize != sizeof(Elf64_Phdr))
        return -1;

    if (ehdr->e_shentsize != sizeof(Elf64_Shdr))
        return -1;

    /* If there is no section header table, then the index should be 0. */
    if (ehdr->e_shnum == 0 && ehdr->e_shstrndx != 0)
        return -1;

    /* If there is a section header table, then the index shouldn't overrun. */
    if (ehdr->e_shnum > 0 && ehdr->e_shstrndx >= ehdr->e_shnum)
        return -1;

    return 0;
}

int elf_dump_ehdr(const void* ehdr)
{
    const Elf64_Ehdr* h = (const Elf64_Ehdr*)ehdr;

    if (!h || _test_header(h) != 0)
        return -1;

    printf("=== elf64_ehdr_t(%lX)\n", (unsigned long)h);

    /* Print e_ident[] */
    printf("e_ident[EI_MAG0]=%02x\n", h->e_ident[EI_MAG0]);
    printf("e_ident[EI_MAG1]=%c\n", h->e_ident[EI_MAG1]);
    printf("e_ident[EI_MAG2]=%c\n", h->e_ident[EI_MAG2]);
    printf("e_ident[EI_MAG3]=%c\n", h->e_ident[EI_MAG3]);

    switch (h->e_ident[EI_CLASS])
    {
        case ELFCLASSNONE:
            printf("e_ident[EI_CLASS]=ELFCLASSNONE\n");
            break;
        case ELFCLASS32:
            printf("e_ident[EI_CLASS]=ELFCLASS32\n");
            break;
        case ELFCLASS64:
            printf("e_ident[EI_CLASS]=ELFCLASS64\n");
            break;
        default:
            printf("e_ident[EI_CLASS]=%02x\n", h->e_ident[EI_CLASS]);
            break;
    }

    switch (h->e_ident[EI_DATA])
    {
        case ELFDATANONE:
            printf("e_ident[EI_DATA]=ELFDATANONE\n");
            break;
        case ELFDATA2LSB:
            printf("e_ident[EI_DATA]=ELFDATA2LSB\n");
            break;
        case ELFDATA2MSB:
            printf("e_ident[EI_DATA]=ELFDATA2MSB\n");
            break;
        default:
            printf("e_ident[EI_DATA]=%02x\n", h->e_ident[EI_DATA]);
            break;
    }

    printf("e_ident[EI_VERSION]=%02x\n", h->e_ident[EI_VERSION]);
    printf("e_ident[EI_PAD]=%02x\n", h->e_ident[EI_PAD]);

    switch (h->e_type)
    {
        case ET_NONE:
            printf("e_type=ET_NONE\n");
            break;
        case ET_REL:
            printf("e_type=ET_REL\n");
            break;
        case ET_EXEC:
            printf("e_type=ET_EXEC\n");
            break;
        case ET_DYN:
            printf("e_type=ET_DYN\n");
            break;
        case ET_CORE:
            printf("e_type=ET_CORE\n");
            break;
        case ET_LOPROC:
            printf("e_type=ET_LOPROC\n");
            break;
        case ET_HIPROC:
            printf("e_type=ET_HIPROC\n");
            break;
        default:
            printf("e_type=%02x\n", h->e_type);
            break;
    }

    switch (h->e_machine)
    {
        case EM_NONE:
            printf("e_machine=EM_NONE\n");
            break;
        case EM_M32:
            printf("e_machine=EM_M32\n");
            break;
        case EM_SPARC:
            printf("e_machine=EM_SPARC\n");
            break;
        case EM_386:
            printf("e_machine=EM_386\n");
            break;
        case EM_68K:
            printf("e_machine=EM_68K\n");
            break;
        case EM_88K:
            printf("e_machine=EM_88K\n");
            break;
        case EM_860:
            printf("e_machine=EM_860\n");
            break;
        case EM_MIPS:
            printf("e_machine=EM_MIPS\n");
            break;
        case EM_X86_64:
            printf("e_machine=EM_X86_64\n");
            break;
        default:
            printf("e_machine=%u\n", h->e_machine);
            break;
    }

    printf("e_version=%u\n", h->e_version);
    printf("e_entry=%lX\n", h->e_entry);
    printf("e_phoff=%lu\n", h->e_phoff);
    printf("e_shoff=%lu\n", h->e_shoff);
    printf("e_flags=%u\n", h->e_flags);
    printf("e_ehsize=%u\n", h->e_ehsize);
    printf("e_phentsize=%u\n", h->e_phentsize);
    printf("e_phnum=%u\n", h->e_phnum);
    printf("e_shentsize=%u\n", h->e_shentsize);
    printf("e_shnum=%u\n", h->e_shnum);
    printf("e_shstrndx=%u\n", h->e_shstrndx);
    printf("\n");

    return 0;
}
