#include "ir.h"

#include <stdio.h>
#include <string.h>

/* need to define
void ir_init(ir_result *ir);
void ir_add_symbol(ir_result *ir, const char *name, const char *lang, const char *file, int line);
void ir_add_dependency(ir_result *ir, const char *from_file, const char *module, const char *type, const char *lang);
*/

void ir_init(ir_result *ir) {
    ir->symbol_count = 0;
    ir->dep_count = 0;
}

ir_symbol *ir_add_symbol(ir_result *ir, const char *name, const char *lang, const char *file, int line) {
    if (ir->symbol_count >= IR_MAX_SYMBOLS) {
        fprintf(stderr, "IR symbol table limit reached, skipping function: %s\n", name);
        return NULL;
    }
    ir_symbol *sym = &ir->symbols[ir->symbol_count++];
    strncpy(sym->name, name, sizeof(sym->name)-1);
    strncpy(sym->lang, lang, sizeof(sym->lang)-1);
    strncpy(sym->file, file, sizeof(sym->file)-1);
    sym->line = line;
    sym->args_count = 0;
    return sym;
}

void ir_symbol_add_args(ir_symbol *sym, const char *arg){
    if(!sym || sym->args_count >= IR_MAX_ARGS) return;
    strncpy(sym->args[sym->args_count++], arg, IR_ARG_LEN - 1);
}

void ir_add_dependency(ir_result *ir, const char *from_file, const char *module, const char *type, const char *lang) {
    if(ir->dep_count >= IR_MAX_DEPS) {
        fprintf(stderr, "IR dependency table limit reached, skipping dependency: %s -> %s\n", from_file, module);
        return;
    }
    ir_dep *dep = &ir->deps[ir->dep_count];
    strncpy(dep->from_file, from_file, sizeof(dep->from_file)-1);
    strncpy(dep->module, module, sizeof(dep->module)-1);
    strncpy(dep->type, type, sizeof(dep->type)-1);
    strncpy(dep->lang, lang, sizeof(dep->lang)-1);
    ir->dep_count++;
}