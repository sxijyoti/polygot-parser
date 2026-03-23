#include "unity.h"
#include "adapters.h"
#include "ir.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

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

static int has_symbol(const ir_result *ir, const char *name, ir_symbol_kind kind, const char *lang) {
    for (int i = 0; i < ir->symbol_count; i++) {
        const ir_symbol *sym = &ir->symbols[i];
        if (strcmp(sym->name, name) == 0 &&
            sym->kind == kind &&
            strcmp(sym->lang, lang) == 0) {
            return 1;
        }
    }
    return 0;
}

void test_detect_supported(void) {
    TEST_ASSERT_EQUAL_INT(PYTHON, detect_lang("file.py"));
    TEST_ASSERT_EQUAL_INT(JS, detect_lang("file.js"));
    TEST_ASSERT_EQUAL_INT(JS, detect_lang("file.mjs"));
    TEST_ASSERT_EQUAL_INT(JS, detect_lang("file.cjs"));
    TEST_ASSERT_EQUAL_INT(RUBY, detect_lang("file.rb"));
}

void test_detect_unsupported(void) {
    TEST_ASSERT_EQUAL_INT(UNSUPPORTED_LANG, detect_lang("file.txt"));
    TEST_ASSERT_EQUAL_INT(UNSUPPORTED_LANG, detect_lang("file"));
    TEST_ASSERT_EQUAL_INT(UNSUPPORTED_LANG, detect_lang("file.jsx"));
    TEST_ASSERT_EQUAL_INT(UNSUPPORTED_LANG, detect_lang("file.PY"));
}

void test_adapter_unsupported(void) {
    ir_result ir;
    ir_init(&ir);

    TEST_ASSERT_EQUAL_INT(-1, lang_adapter("file.txt", UNSUPPORTED_LANG, &ir));
}

void test_py_adapter(void) {
    char py_path[512];
    example_path(py_path, sizeof(py_path), "example.py");

    ir_result ir;
    ir_init(&ir);

    TEST_ASSERT_EQUAL_INT(0, lang_adapter(py_path, PYTHON, &ir));
    TEST_ASSERT_TRUE(ir.symbol_count >= 1);
    TEST_ASSERT_TRUE(has_symbol(&ir, "sum", IR_SYMBOL_FUNCTION, "py"));
    TEST_ASSERT_TRUE(has_symbol(&ir, "Calculator", IR_SYMBOL_CLASS, "py"));
    TEST_ASSERT_TRUE(has_symbol(&ir, "DATA", IR_SYMBOL_OBJECT, "py"));
}

void test_js_adapter(void) {
    char js_path[512];
    example_path(js_path, sizeof(js_path), "example.js");

    ir_result ir;
    ir_init(&ir);

    TEST_ASSERT_EQUAL_INT(0, lang_adapter(js_path, JS, &ir));
    TEST_ASSERT_TRUE(ir.symbol_count >= 1);
    TEST_ASSERT_TRUE(has_symbol(&ir, "run", IR_SYMBOL_FUNCTION, "js"));
    TEST_ASSERT_TRUE(has_symbol(&ir, "Runner", IR_SYMBOL_CLASS, "js"));
    TEST_ASSERT_TRUE(has_symbol(&ir, "settings", IR_SYMBOL_OBJECT, "js"));
}

void test_rb_adapter(void) {
    char rb_path[512];
    example_path(rb_path, sizeof(rb_path), "example.rb");

    ir_result ir;
    ir_init(&ir);

    TEST_ASSERT_EQUAL_INT(0, lang_adapter(rb_path, RUBY, &ir));
    TEST_ASSERT_TRUE(ir.symbol_count >= 1);
    TEST_ASSERT_TRUE(has_symbol(&ir, "greet", IR_SYMBOL_FUNCTION, "rb"));
    TEST_ASSERT_TRUE(has_symbol(&ir, "Greeter", IR_SYMBOL_CLASS, "rb"));
    TEST_ASSERT_TRUE(has_symbol(&ir, "settings", IR_SYMBOL_OBJECT, "rb"));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_detect_supported);
    RUN_TEST(test_detect_unsupported);
    RUN_TEST(test_adapter_unsupported);
    RUN_TEST(test_py_adapter);
    RUN_TEST(test_js_adapter);
    RUN_TEST(test_rb_adapter);
    return UNITY_END();
}
