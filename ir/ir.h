#ifndef IR_H
#define IR_H

#include <stddef.h>

#define IR_MAX_SYMBOLS 512
#define IR_MAX_DEPS    1024
#define IR_MAX_ARGS    16
#define IR_ARG_LEN     64

typedef struct {
    char name[64];
    char lang[8]; 
    char file[256];
    int line;
    char args[IR_MAX_ARGS][IR_ARG_LEN];
    int args_count;
} ir_symbol;

// for dependency graph
typedef struct{
    char from_file[256];
    char module[256];
    char type[24];
    char lang[8];
} ir_dep;

typedef struct {
    ir_symbol symbols[IR_MAX_SYMBOLS];
    int       symbol_count;

    ir_dep    deps[IR_MAX_DEPS];
    int       dep_count;
} ir_result;

//  api
void ir_init(ir_result *ir);
ir_symbol *ir_add_symbol(ir_result *ir, const char *name, const char *lang, const char *file, int line);
void ir_symbol_add_args(ir_symbol *sym, const char *arg);
void ir_add_dependency(ir_result *ir, const char *from_file, const char *module, const char *type, const char *lang);

#endif