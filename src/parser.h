#ifndef PARSER_H
#define PARSER_H

typedef struct polyparser_result polyparser_result;

polyparser_result *polyparser_parse_file(const char *fpath);
polyparser_result *polyparser_parse_dir(const char *dirpath);
polyparser_result *polyparser_parse_files(const char **fpaths, int count);
char *polyparser_to_json(const polyparser_result *res);
void polyparser_free_json(char *json);
void polyparser_free_res(polyparser_result *res);

#endif