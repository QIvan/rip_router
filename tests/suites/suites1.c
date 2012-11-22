#include <stdio.h>
#include <stdlib.h>
#include <CUnit/Basic.h>

#include "utils.h"
#include "main.h"

TEST_FUNCT(foo) {
     /* Фейковый код */
    CU_ASSERT_EQUAL(add(1, 122), 123);
}
TEST_FUNCT(foo2) {
    /* Фейковый код */
    CU_ASSERT_EQUAL(1, 1);
}

void runSuite(void) {
    CU_pSuite suite = CUnitCreateSuite("Suite1");
    if (suite) {
        ADD_SUITE_TEST(suite, foo)
        ADD_SUITE_TEST(suite, foo2)
    }
}
