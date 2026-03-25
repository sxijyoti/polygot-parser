#include "unity.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#ifndef PROJECT_SOURCE_DIR
#define PROJECT_SOURCE_DIR "."
#endif

#ifndef PARSER_BIN_PATH
#define PARSER_BIN_PATH "./polygot_parser"
#endif

void setUp(void) {}
void tearDown(void) {}

static int run_cmd(const char *cmd) {
    int status = system(cmd);
    if (status == -1) return -1;
    if (WIFEXITED(status)) return WEXITSTATUS(status);
    return status;
}

static char *read_text_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buf = (char *)malloc((size_t)size + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }

    fread(buf, 1, (size_t)size, f);
    buf[size] = '\0';
    fclose(f);
    return buf;
}

void test_help(void) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "\"%s\" --help > /dev/null 2>&1", PARSER_BIN_PATH);

    int code = run_cmd(cmd);
    TEST_ASSERT_EQUAL_INT(0, code);
}

void test_no_args(void) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "\"%s\" > /dev/null 2>&1", PARSER_BIN_PATH);

    int code = run_cmd(cmd);
    TEST_ASSERT_NOT_EQUAL(0, code);
}

void test_missing_file_for_f(void) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "\"%s\" -f > /dev/null 2>&1", PARSER_BIN_PATH);

    int code = run_cmd(cmd);
    TEST_ASSERT_NOT_EQUAL(0, code);
}

void test_f_two_files(void) {
    char cmd[2048];
    snprintf(
        cmd,
        sizeof(cmd),
        "\"%s\" -f \"%s/examples/example.py\" \"%s/examples/example.js\" > /dev/null 2>&1",
        PARSER_BIN_PATH,
        PROJECT_SOURCE_DIR,
        PROJECT_SOURCE_DIR
    );

    int code = run_cmd(cmd);
    TEST_ASSERT_EQUAL_INT(0, code);
}

void test_d_mixed(void) {
    char cmd[2048];
    snprintf(
        cmd,
        sizeof(cmd),
        "\"%s\" -d \"%s/examples\" > /dev/null 2>&1",
        PARSER_BIN_PATH,
        PROJECT_SOURCE_DIR
    );

    int code = run_cmd(cmd);
    TEST_ASSERT_EQUAL_INT(0, code);
}

void test_o_file(void) {
    char out_path[1024];
    snprintf(out_path, sizeof(out_path), "%s/build/test_cli_output.json", PROJECT_SOURCE_DIR);

    char cmd[2048];
    snprintf(
        cmd,
        sizeof(cmd),
        "\"%s\" -d \"%s/examples\" -o \"%s\" > /dev/null 2>&1",
        PARSER_BIN_PATH,
        PROJECT_SOURCE_DIR,
        out_path
    );

    int code = run_cmd(cmd);
    TEST_ASSERT_EQUAL_INT(0, code);

    char *txt = read_text_file(out_path);
    TEST_ASSERT_NOT_NULL(txt);
    TEST_ASSERT_NOT_NULL(strstr(txt, "\"languages\""));
    TEST_ASSERT_NOT_NULL(strstr(txt, "\"py\""));
    TEST_ASSERT_NOT_NULL(strstr(txt, "\"js\""));
    TEST_ASSERT_NOT_NULL(strstr(txt, "\"rb\""));
    TEST_ASSERT_NOT_NULL(strstr(txt, "example.py"));
    TEST_ASSERT_NOT_NULL(strstr(txt, "example.rb"));

    free(txt);
    remove(out_path);
}

void test_conflict_f_d(void) {
    char cmd[2048];
    snprintf(
        cmd,
        sizeof(cmd),
        "\"%s\" -f \"%s/examples/example.py\" -d \"%s/examples\" > /dev/null 2>&1",
        PARSER_BIN_PATH,
        PROJECT_SOURCE_DIR,
        PROJECT_SOURCE_DIR
    );

    int code = run_cmd(cmd);
    TEST_ASSERT_NOT_EQUAL(0, code);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_help);
    RUN_TEST(test_no_args);
    RUN_TEST(test_missing_file_for_f);
    RUN_TEST(test_f_two_files);
    RUN_TEST(test_d_mixed);
    RUN_TEST(test_o_file);
    RUN_TEST(test_conflict_f_d);
    return UNITY_END();
}
