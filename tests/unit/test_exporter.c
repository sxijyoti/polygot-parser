#include "unity.h"
#include "mc_export.h"
#include "ir.h"
#include "graph.h"

#include <string.h>
#include <stdlib.h>

void setUp(void) {}
void tearDown(void) {}

void test_export_json(void) {
    ir_result ir;
    ir_init(&ir);

    ir_symbol *sym = ir_add_symbol(&ir, "foo", IR_SYMBOL_FUNCTION, "py", "x.py", 1);
    TEST_ASSERT_NOT_NULL(sym);
    ir_symbol_set_owner(sym, "Bar");
    ir_symbol_add_args(sym, "x");
    ir_symbol_add_args(sym, "y");

    TEST_ASSERT_NOT_NULL(ir_add_symbol(&ir, "Bar", IR_SYMBOL_CLASS, "py", "x.py", 1));

    ir_add_dependency(&ir, "x.js", "./x.py", "require", "js");

    grp g;
    grp_builder(&g, &ir);

    char *json = mc_export_json(&ir, &g);
    TEST_ASSERT_NOT_NULL(json);
    TEST_ASSERT_NOT_NULL(strstr(json, "\"languages\""));
    TEST_ASSERT_NOT_NULL(strstr(json, "\"foo\""));
    TEST_ASSERT_NOT_NULL(strstr(json, "\"functions\""));
    TEST_ASSERT_NOT_NULL(strstr(json, "\"classes\""));
    TEST_ASSERT_NOT_NULL(strstr(json, "\"objects\""));
    TEST_ASSERT_NOT_NULL(strstr(json, "\"graph\""));
    TEST_ASSERT_NOT_NULL(strstr(json, "\"edges\""));
    TEST_ASSERT_NOT_NULL(strstr(json, "\"x.js\""));
    TEST_ASSERT_NOT_NULL(strstr(json, "\"member_of\""));

    free(json);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_export_json);
    return UNITY_END();
}
