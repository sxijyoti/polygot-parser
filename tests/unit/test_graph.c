#include "unity.h"
#include "graph.h"
#include "ir.h"

#include <string.h>

void setUp(void) {}
void tearDown(void) {}

void test_build_edges(void) {
    ir_result ir;
    ir_init(&ir);

    ir_add_dependency(&ir, "a.js", "./b.py", "require", "js");
    ir_add_dependency(&ir, "c.rb", "json", "require", "rb");

    grp g;
    grp_builder(&g, &ir);

    TEST_ASSERT_EQUAL_INT(2, g.edge_count);
    TEST_ASSERT_EQUAL_STRING("a.js", g.edges[0].from);
    TEST_ASSERT_EQUAL_STRING("./b.py", g.edges[0].to);
    TEST_ASSERT_EQUAL_INT(GRP_NODE_FILE, g.edges[0].from_kind);
    TEST_ASSERT_EQUAL_INT(GRP_NODE_MODULE, g.edges[0].to_kind);
    TEST_ASSERT_EQUAL_INT(GRP_EDGE_REQUIRE, g.edges[0].rel_kind);
    TEST_ASSERT_EQUAL_STRING("js", g.edges[0].lang);

    TEST_ASSERT_EQUAL_STRING("c.rb", g.edges[1].from);
    TEST_ASSERT_EQUAL_STRING("json", g.edges[1].to);
    TEST_ASSERT_EQUAL_INT(GRP_EDGE_REQUIRE, g.edges[1].rel_kind);
}

void test_build_empty(void) {
    ir_result ir;
    ir_init(&ir);

    grp g;
    grp_builder(&g, &ir);

    TEST_ASSERT_EQUAL_INT(0, g.edge_count);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_build_edges);
    RUN_TEST(test_build_empty);
    return UNITY_END();
}
