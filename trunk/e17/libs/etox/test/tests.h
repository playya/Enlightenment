#ifndef __ETOX_TEST_TESTS_H__
#define __ETOX_TEST_TESTS_H__

/*
 * Basic test prototypes
 */
Evas_List *basic_tests();
void test_basic_init();
void test_basic_get();
void test_basic_set();
void test_basic_append();

/*
 * Style tests prototypes
 */
Evas_List *style_tests();
void test_style_init();
void test_style_bold();
void test_style_outline();
void test_style_raised();
void test_style_shadow();

/*
 * Callback tests prototypes
 */
Evas_List *callback_tests();
void test_callback_init();
/* void test_callback_add(); */

#endif				/* __ETOX_TEST_TESTS_H__ */
