#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>
#include "debugger.h"
#include "log.h"

static FILE *open_elf(debugger_t *dbg)
{
    return fopen(dbg->target, "rb");
}

static int read_elf(debugger_t *dbg,
                    Elf64_Ehdr *ehdr,
                    Elf64_Shdr **shdrs_out,
                    char       **shstrtab_out)
{
    FILE *f = open_elf(dbg);
    if (!f) {
        log_err("cannot open '%s'\n", dbg->target);
        return -1;
    }

    if (fread(ehdr, sizeof(*ehdr), 1, f) != 1) {
        log_err("cannot read ELF header\n");
        fclose(f);
        return -1;
    }

    if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0) {
        log_err("'%s' is not an ELF file\n", dbg->target);
        fclose(f);
        return -1;
    }

    if (ehdr->e_ident[EI_CLASS] != ELFCLASS64) {
        log_err("only 64-bit ELF is supported\n");
        fclose(f);
        return -1;
    }

    Elf64_Shdr *shdrs = malloc(ehdr->e_shnum * sizeof(Elf64_Shdr));
    if (!shdrs) {
        log_err("out of memory\n");
        fclose(f);
        return -1;
    }

    if (fseek(f, (long)ehdr->e_shoff, SEEK_SET) != 0 ||
        fread(shdrs, sizeof(Elf64_Shdr), ehdr->e_shnum, f) != ehdr->e_shnum) {
        log_err("cannot read section headers\n");
        free(shdrs);
        fclose(f);
        return -1;
    }

    Elf64_Shdr *shstr = &shdrs[ehdr->e_shstrndx];
    char *shstrtab = malloc(shstr->sh_size);
    if (!shstrtab) {
        log_err("out of memory\n");
        free(shdrs);
        fclose(f);
        return -1;
    }

    if (fseek(f, (long)shstr->sh_offset, SEEK_SET) != 0 ||
        fread(shstrtab, 1, shstr->sh_size, f) != shstr->sh_size) {
        log_err("cannot read shstrtab\n");
        free(shstrtab);
        free(shdrs);
        fclose(f);
        return -1;
    }

    *shdrs_out     = shdrs;
    *shstrtab_out  = shstrtab;
    fclose(f);
    return 0;
}

uint64_t dbg_sym_lookup(debugger_t *dbg, const char *name)
{
    Elf64_Ehdr  ehdr;
    Elf64_Shdr *shdrs     = NULL;
    char       *shstrtab  = NULL;

    if (read_elf(dbg, &ehdr, &shdrs, &shstrtab) != 0)
        return 0;

    if (ehdr.e_type == ET_DYN)
        log_warn("PIE binary — addresses are relative to load base\n");

    Elf64_Shdr *symtab_sh = NULL;
    Elf64_Shdr *strtab_sh = NULL;

    for (int i = 0; i < ehdr.e_shnum; i++) {
        const char *sname = shstrtab + shdrs[i].sh_name;
        if (shdrs[i].sh_type == SHT_SYMTAB && strcmp(sname, ".symtab") == 0)
            symtab_sh = &shdrs[i];
        if (shdrs[i].sh_type == SHT_STRTAB && strcmp(sname, ".strtab") == 0)
            strtab_sh = &shdrs[i];
    }

    if (!symtab_sh || !strtab_sh) {
        log_warn("no .symtab/.strtab found in '%s'\n", dbg->target);
        free(shdrs);
        free(shstrtab);
        return 0;
    }

    FILE *f = open_elf(dbg);
    if (!f) {
        free(shdrs);
        free(shstrtab);
        return 0;
    }

    size_t sym_count = symtab_sh->sh_size / sizeof(Elf64_Sym);
    Elf64_Sym *syms  = malloc(symtab_sh->sh_size);
    char      *strtab = malloc(strtab_sh->sh_size);

    if (!syms || !strtab) {
        log_err("out of memory\n");
        free(syms);
        free(strtab);
        fclose(f);
        free(shdrs);
        free(shstrtab);
        return 0;
    }

    fseek(f, (long)symtab_sh->sh_offset, SEEK_SET);
    fread(syms, sizeof(Elf64_Sym), sym_count, f);
    fseek(f, (long)strtab_sh->sh_offset, SEEK_SET);
    fread(strtab, 1, strtab_sh->sh_size, f);
    fclose(f);

    uint64_t result = 0;
    for (size_t i = 0; i < sym_count; i++) {
        const char *sname = strtab + syms[i].st_name;
        if (strcmp(sname, name) == 0) {
            result = syms[i].st_value;
            break;
        }
    }

    if (result == 0)
        log_warn("symbol '%s' not found\n", name);

    free(syms);
    free(strtab);
    free(shdrs);
    free(shstrtab);
    return result;
}

void dbg_sym_list(debugger_t *dbg)
{
    Elf64_Ehdr  ehdr;
    Elf64_Shdr *shdrs    = NULL;
    char       *shstrtab = NULL;

    if (read_elf(dbg, &ehdr, &shdrs, &shstrtab) != 0)
        return;

    Elf64_Shdr *symtab_sh = NULL;
    Elf64_Shdr *strtab_sh = NULL;

    for (int i = 0; i < ehdr.e_shnum; i++) {
        const char *sname = shstrtab + shdrs[i].sh_name;
        if (shdrs[i].sh_type == SHT_SYMTAB && strcmp(sname, ".symtab") == 0)
            symtab_sh = &shdrs[i];
        if (shdrs[i].sh_type == SHT_STRTAB && strcmp(sname, ".strtab") == 0)
            strtab_sh = &shdrs[i];
    }

    if (!symtab_sh || !strtab_sh) {
        log_warn("no .symtab/.strtab found in '%s'\n", dbg->target);
        free(shdrs);
        free(shstrtab);
        return;
    }

    FILE *f = open_elf(dbg);
    if (!f) {
        free(shdrs);
        free(shstrtab);
        return;
    }

    size_t sym_count = symtab_sh->sh_size / sizeof(Elf64_Sym);
    Elf64_Sym *syms  = malloc(symtab_sh->sh_size);
    char      *strtab = malloc(strtab_sh->sh_size);

    if (!syms || !strtab) {
        log_err("out of memory\n");
        free(syms);
        free(strtab);
        fclose(f);
        free(shdrs);
        free(shstrtab);
        return;
    }

    fseek(f, (long)symtab_sh->sh_offset, SEEK_SET);
    fread(syms, sizeof(Elf64_Sym), sym_count, f);
    fseek(f, (long)strtab_sh->sh_offset, SEEK_SET);
    fread(strtab, 1, strtab_sh->sh_size, f);
    fclose(f);

    int found = 0;
    for (size_t i = 0; i < sym_count; i++) {
        if (ELF64_ST_TYPE(syms[i].st_info) == STT_FUNC && syms[i].st_value != 0) {
            log_msg("0x%016lx  %s\n", syms[i].st_value, strtab + syms[i].st_name);
            found++;
        }
    }
    if (found == 0)
        log_warn("no function symbols found\n");

    free(syms);
    free(strtab);
    free(shdrs);
    free(shstrtab);
}
