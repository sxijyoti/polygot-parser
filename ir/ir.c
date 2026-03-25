#include "ir.h"

#include <stdio.h>
#include <string.h>

/* need to define
void ir_init(ir_result *ir);
void ir_add_symbol(ir_result *ir, const char *name, ir_symbol_kind kind, const char *lang, const char *file, int line);
void ir_add_dependency(ir_result *ir, const char *from_file, const char *module, const char *type, const char *lang);
*/

static void safe_copy(char *dst, size_t cap, const char *src) {
    if (!dst || cap == 0) return;
    if (!src) {
        dst[0] = '\0';
        return;
    }
    strncpy(dst, src, cap - 1);
    dst[cap - 1] = '\0';
}

void ir_init(ir_result *ir) {
    ir->symbol_count = 0;
    ir->dep_count = 0;
}

ir_symbol *ir_add_symbol(ir_result *ir, const char *name, ir_symbol_kind kind, const char *lang, const char *file, int line) {
    if (ir->symbol_count >= IR_MAX_SYMBOLS) {
        fprintf(stderr, "IR symbol table limit reached, skipping function: %s\n", name);
        return NULL;
    }
    ir_symbol *sym = &ir->symbols[ir->symbol_count++];
    memset(sym, 0, sizeof(*sym));
    safe_copy(sym->name, sizeof(sym->name), name);
    sym->kind = kind;
    sym->is_exported = 1;
    safe_copy(sym->lang, sizeof(sym->lang), lang);
    safe_copy(sym->file, sizeof(sym->file), file);
    sym->line = line;
    sym->args_count = 0;
    return sym;
}

void ir_symbol_set_owner(ir_symbol *sym, const char *owner) {
    if (!sym) return;
    safe_copy(sym->owner, sizeof(sym->owner), owner);
}

void ir_symbol_add_args(ir_symbol *sym, const char *arg){
    if(!sym || sym->args_count >= IR_MAX_ARGS) return;
    safe_copy(sym->args[sym->args_count++], IR_ARG_LEN, arg);
}

void ir_add_dependency(ir_result *ir, const char *from_file, const char *module, const char *type, const char *lang) {
    if(ir->dep_count >= IR_MAX_DEPS) {
        fprintf(stderr, "IR dependency table limit reached, skipping dependency: %s -> %s\n", from_file, module);
        return;
    }
    ir_dep *dep = &ir->deps[ir->dep_count];
    memset(dep, 0, sizeof(*dep));
    safe_copy(dep->from_file, sizeof(dep->from_file), from_file);
    safe_copy(dep->module, sizeof(dep->module), module);
    safe_copy(dep->type, sizeof(dep->type), type);
    safe_copy(dep->lang, sizeof(dep->lang), lang);
    ir->dep_count++;
}