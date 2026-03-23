#include "unity.h"
#include "parser.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef PROJECT_SOURCE_DIR
#define PROJECT_SOURCE_DIR "."
#endif

void setUp(void) {}
void tearDown(void) {}

static int file_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0;
}

static void example_path(char *out, size_t cap, const char *relative) {
    snprintf(out, cap, "%s/examples/%s", PROJECT_SOURCE_DIR, relative);
    if (file_exists(out)) return;

    snprintf(out, cap, "../../examples/%s", relative);
    if (file_exists(out)) return;

    snprintf(out, cap, "examples/%s", relative);
}

static int write_text(const char *path, const char *txt) {
    FILE *f = fopen(path, "w");
    if (!f) return 0;
    fputs(txt, f);
    fclose(f);
    return 1;
}

void test_parse_file_ok(void) {
    char py_path[512];
    example_path(py_path, sizeof(py_path), "example.py");

    polyparser_result *res = polyparser_parse_file(py_path);
    TEST_ASSERT_NOT_NULL(res);

    char *json = polyparser_to_json(res);
    TEST_ASSERT_NOT_NULL(json);
    TEST_ASSERT_NOT_NULL(strstr(json, "\"py\""));
    TEST_ASSERT_NOT_NULL(strstr(json, "\"sum\""));

    polyparser_free_json(json);
    polyparser_free_res(res);
}

void test_parse_file_bad_ext(void) {
    char err_path[512];
    example_path(err_path, sizeof(err_path), "error.txt");

    polyparser_result *res = polyparser_parse_file(err_path);
    TEST_ASSERT_NULL(res);
}

void test_parse_files_empty(void) {
    polyparser_result *res = polyparser_parse_files(NULL, 0);
    TEST_ASSERT_NULL(res);
}

void test_parse_files_mixed(void) {
    char py_path[512];
    char js_path[512];
    char err_path[512];
    example_path(py_path, sizeof(py_path), "example.py");
    example_path(js_path, sizeof(js_path), "example.js");
    example_path(err_path, sizeof(err_path), "error.txt");

    const char *files[] = {py_path, js_path, err_path};
    polyparser_result *res = polyparser_parse_files(files, 3);

    TEST_ASSERT_NOT_NULL(res);

    char *json = polyparser_to_json(res);
    TEST_ASSERT_NOT_NULL(json);
    TEST_ASSERT_NOT_NULL(strstr(json, "\"py\""));
    TEST_ASSERT_NOT_NULL(strstr(json, "\"js\""));

    polyparser_free_json(json);
    polyparser_free_res(res);
}

void test_parse_dir_mixed(void) {
    char tmpl[] = "/tmp/polygot_mixed_XXXXXX";
    char *dir_path = mkdtemp(tmpl);
    TEST_ASSERT_NOT_NULL(dir_path);

    char py_file[512], js_file[512], rb_file[512], txt_file[512], hidden_file[512], subdir[512], sub_file[512];
    snprintf(py_file, sizeof(py_file), "%s/a.py", dir_path);
    snprintf(js_file, sizeof(js_file), "%s/b.js", dir_path);
    snprintf(rb_file, sizeof(rb_file), "%s/c.rb", dir_path);
    snprintf(txt_file, sizeof(txt_file), "%s/d.txt", dir_path);
    snprintf(hidden_file, sizeof(hidden_file), "%s/.hidden.py", dir_path);
    snprintf(subdir, sizeof(subdir), "%s/subdir", dir_path);
    snprintf(sub_file, sizeof(sub_file), "%s/inner.py", subdir);

    TEST_ASSERT_EQUAL_INT(0, mkdir(subdir, 0700));
    TEST_ASSERT_TRUE(write_text(py_file, "def mixed_py(a):\n    return a\n"));
    TEST_ASSERT_TRUE(write_text(js_file, "function mixedJs() { return 1; }\n"));
    TEST_ASSERT_TRUE(write_text(rb_file, "def mixed_rb(x)\n  x\nend\n"));
    TEST_ASSERT_TRUE(write_text(txt_file, "skip\n"));
    TEST_ASSERT_TRUE(write_text(hidden_file, "def hidden_py():\n    return 0\n"));
    TEST_ASSERT_TRUE(write_text(sub_file, "def inner_py():\n    return 42\n"));

    polyparser_result *res = polyparser_parse_dir(dir_path);
    TEST_ASSERT_NOT_NULL(res);

    char *json = polyparser_to_json(res);
    TEST_ASSERT_NOT_NULL(json);

    TEST_ASSERT_NOT_NULL(strstr(json, "\"py\""));
    TEST_ASSERT_NOT_NULL(strstr(json, "\"js\""));
    TEST_ASSERT_NOT_NULL(strstr(json, "\"rb\""));
    TEST_ASSERT_NULL(strstr(json, "hidden_py"));
    TEST_ASSERT_NOT_NULL(strstr(json, "inner_py"));

    polyparser_free_json(json);
    polyparser_free_res(res);

    remove(sub_file);
    rmdir(subdir);
    remove(hidden_file);
    remove(txt_file);
    remove(rb_file);
    remove(js_file);
    remove(py_file);
    rmdir(dir_path);
}

void test_parse_dir_empty(void) {
    char tmpl[] = "/tmp/polygot_empty_XXXXXX";
    char *dir_path = mkdtemp(tmpl);
    TEST_ASSERT_NOT_NULL(dir_path);

    polyparser_result *res = polyparser_parse_dir(dir_path);
    TEST_ASSERT_NOT_NULL(res);

    char *json = polyparser_to_json(res);
    TEST_ASSERT_NOT_NULL(json);
    TEST_ASSERT_NOT_NULL(strstr(json, "\"languages\": {}"));
    TEST_ASSERT_NOT_NULL(strstr(json, "\"edges\": []"));

    polyparser_free_json(json);
    polyparser_free_res(res);

    rmdir(dir_path);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_parse_file_ok);
    RUN_TEST(test_parse_file_bad_ext);
    RUN_TEST(test_parse_files_empty);
    RUN_TEST(test_parse_files_mixed);
    RUN_TEST(test_parse_dir_mixed);
    RUN_TEST(test_parse_dir_empty);
    return UNITY_END();
}
