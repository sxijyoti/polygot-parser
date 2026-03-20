#include "ir.h"

#include <stdio.h>
#include <string.h>

/* need to define
void ir_init(ir_result *ir);
void ir_add_function(ir_result *ir, const char *name, const char *lang, const char *file);
void ir_add_dependency(ir_result *ir, const char *from_file, const char *module); 
*/

void ir_init(ir_result *ir) {
    ir->symbol_count = 0;
    ir->dep_count = 0;
}

void ir_add_function(ir_result *ir, const char *name, const char *lang, const char *file) {
    if (ir->symbol_count >= IR_SYMBOLS) {
        fprintf(stderr, "IR symbol table limit reached, skipping function: %s\n", name);
        return;
    }
    ir_symbol *syb = &ir->symbols[ir->symbol_count++];
    strncpy(syb->type, "function", sizeof(syb->type));
    strncpy(syb->name, name, sizeof(syb->name));
    strncpy(syb->language, lang, sizeof(syb->language));
    strncpy(syb->file, file, sizeof(syb->file));

}

void ir_add_dependency(ir_result *ir, const char *from_file, const char *module){
    if(ir->dep_count >= IR_DEPS) {
        fprintf(stderr, "IR dependency table limit reached, skipping dependency: %s -> %s\n", from_file, module);
        return;
    }
    strncpy(ir->deps[ir->dep_count][0], from_file, sizeof(ir->deps[0][0]));
    strncpy(ir->deps[ir->dep_count][1], module, sizeof(ir->deps[0][1]));
    ir->dep_count++;
}