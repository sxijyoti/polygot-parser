#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILES 512

// cli logic
static void print_usage(const char *prog) {
    fprintf(stderr,
        "\nWelcome to polyglot-parser! :D\n\n"
        "Currently supported languages: Python(.py), JavaScript(.js, .mjs, .cjs), Ruby(.rb)\n"
        "Usage:\n"
        "  %s -f <file>           parse one file\n"
        "  %s -f <file1> <file2>  parse more than one file\n"
        "  %s -d <directory>      parse all supported files in a directory\n"
        "\n"
        "Options:\n"
        "  -o <output.json>        write JSON to file (default: stdout)\n"
        "  -h, --help              show this help\n"
        "\n",
        prog, prog, prog);
}

int main(int argc, char **argv) {
    const char *file_args[MAX_FILES] = {0};
    int         file_count = 0;
    const char *dir_arg   = NULL;
    const char *out_arg   = NULL;

    // parse arguments
    for (int i = 1; i < argc; i++) {
        if ((strcmp(argv[i], "-h") == 0) ||
            (strcmp(argv[i], "--help") == 0)) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-f") == 0) {
            if (dir_arg) {
                fprintf(stderr, "[Error] -f and -d can not be used together\n\n");
                print_usage(argv[0]);
                return 1;
            }
            if (i + 1 >= argc) {
                fprintf(stderr, "[Error] -f requires at least one file\n\n");
                print_usage(argv[0]);
                return 1;
            }
            while (++i < argc && argv[i][0] != '-') {
                if (file_count < (int)(sizeof(file_args) / sizeof(file_args[0]))) {
                    file_args[file_count++] = argv[i];
                } else {
                    fprintf(stderr, "[Warning] too many files. Skipping %s\n", argv[i]);
                }
            }
            i--; 
            
        } else if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
            dir_arg = argv[++i];
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            out_arg = argv[++i];
        } else {
            fprintf(stderr, "[Error] unknown argument: %s\n\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }

    if (file_count == 0 && !dir_arg) {
        fprintf(stderr, "[Error] incomplete arguments\n\n");
        print_usage(argv[0]);
        return 1;
    }

    // parse
    polyparser_result *res = NULL;

    if (file_count > 0) {
        res = polyparser_parse_files(file_args, file_count);
        if (!res) {
            fprintf(stderr, "[Error] Failed to parse provided files\n");
            return 1;
        }
    } else {
        res = polyparser_parse_dir(dir_arg);
        if (!res) {
            fprintf(stderr, "[Error] Failed to parse directory: %s\n", dir_arg);
            return 1;
        }
    }

    // serialize
    char *json = polyparser_to_json(res);
    if (!json) {
        fprintf(stderr, "[Error] JSON serialisation failed\n");
        polyparser_free_res(res);
        return 1;
    }

    if (out_arg) {
        FILE *f = fopen(out_arg, "w");
        if (!f) {
            fprintf(stderr, "[Error] Cannot open output file: %s\n", out_arg);
            polyparser_free_json(json);
            polyparser_free_res(res);
            return 1;
        }
        fputs(json, f);
        fputc('\n', f);
        fclose(f);
        fprintf(stderr, "Output written to %s\n", out_arg);
    } else {
        puts(json);
    }

    polyparser_free_json(json);
    polyparser_free_res(res);
    return 0;
}
