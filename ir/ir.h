#ifndef IR_H
#define IR_H

#include <stddef.h>

#define IR_SYMBOLS 512
#define IR_DEPS    512

typedef struct {
    char type[16];     
    char name[64];
    char language[8]; 
    char file[256];
} ir_symbol;

typedef struct {
    ir_symbol symbols[IR_SYMBOLS];
    int       symbol_count;

    // dep
    char      deps[IR_DEPS][2][256];
    int       dep_count;
} ir_result;

//  api
void ir_init(ir_result *ir);
void ir_add_function(ir_result *ir, const char *name, const char *lang, const char *file);
void ir_add_dependency(ir_result *ir, const char *from_file, const char *module);

#endif