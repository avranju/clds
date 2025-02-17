// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdbool.h>
#include <inttypes.h>
#include <stdlib.h>

#include "macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"

#include "real_gballoc_ll.h"

#include "umock_c/umock_c.h"
#include "umock_c/umocktypes_stdint.h"
#include "umock_c/umocktypes_bool.h"
#include "umock_c/umock_c_negative_tests.h"

#include "c_pal/interlocked.h"

#define ENABLE_MOCKS

#include "c_pal/gballoc_hl.h"
#include "c_pal/gballoc_hl_redirect.h"

#include "clds/clds_sorted_list.h"
#include "clds/clds_st_hash_set.h"
#include "clds/clds_hazard_pointers.h"

#undef ENABLE_MOCKS

#include "real_gballoc_hl.h"

#include "../reals/real_clds_st_hash_set.h"
#include "../reals/real_clds_hazard_pointers.h"
#include "../reals/real_clds_sorted_list.h"

#include "clds/clds_hash_table.h"

TEST_DEFINE_ENUM_TYPE(CLDS_SORTED_LIST_INSERT_RESULT, CLDS_SORTED_LIST_INSERT_RESULT_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(CLDS_SORTED_LIST_INSERT_RESULT, CLDS_SORTED_LIST_INSERT_RESULT_VALUES);
TEST_DEFINE_ENUM_TYPE(CLDS_SORTED_LIST_DELETE_RESULT, CLDS_SORTED_LIST_DELETE_RESULT_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(CLDS_SORTED_LIST_DELETE_RESULT, CLDS_SORTED_LIST_DELETE_RESULT_VALUES);
TEST_DEFINE_ENUM_TYPE(CLDS_SORTED_LIST_REMOVE_RESULT, CLDS_SORTED_LIST_REMOVE_RESULT_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(CLDS_SORTED_LIST_REMOVE_RESULT, CLDS_SORTED_LIST_REMOVE_RESULT_VALUES);
TEST_DEFINE_ENUM_TYPE(CLDS_SORTED_LIST_SET_VALUE_RESULT, CLDS_SORTED_LIST_SET_VALUE_RESULT_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(CLDS_SORTED_LIST_SET_VALUE_RESULT, CLDS_SORTED_LIST_SET_VALUE_RESULT_VALUES);

TEST_DEFINE_ENUM_TYPE(CLDS_HASH_TABLE_INSERT_RESULT, CLDS_HASH_TABLE_INSERT_RESULT_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(CLDS_HASH_TABLE_INSERT_RESULT, CLDS_HASH_TABLE_INSERT_RESULT_VALUES);
TEST_DEFINE_ENUM_TYPE(CLDS_HASH_TABLE_DELETE_RESULT, CLDS_HASH_TABLE_DELETE_RESULT_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(CLDS_HASH_TABLE_DELETE_RESULT, CLDS_HASH_TABLE_DELETE_RESULT_VALUES);
TEST_DEFINE_ENUM_TYPE(CLDS_HASH_TABLE_REMOVE_RESULT, CLDS_HASH_TABLE_REMOVE_RESULT_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(CLDS_HASH_TABLE_REMOVE_RESULT, CLDS_HASH_TABLE_REMOVE_RESULT_VALUES);
TEST_DEFINE_ENUM_TYPE(CLDS_HASH_TABLE_SET_VALUE_RESULT, CLDS_HASH_TABLE_SET_VALUE_RESULT_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(CLDS_HASH_TABLE_SET_VALUE_RESULT, CLDS_HASH_TABLE_SET_VALUE_RESULT_VALUES);
TEST_DEFINE_ENUM_TYPE(CLDS_HASH_TABLE_SNAPSHOT_RESULT, CLDS_HASH_TABLE_SNAPSHOT_RESULT_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(CLDS_HASH_TABLE_SNAPSHOT_RESULT, CLDS_HASH_TABLE_SNAPSHOT_RESULT_VALUES);
TEST_DEFINE_ENUM_TYPE(CLDS_CONDITION_CHECK_RESULT, CLDS_CONDITION_CHECK_RESULT_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(CLDS_CONDITION_CHECK_RESULT, CLDS_CONDITION_CHECK_RESULT_VALUES);

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

static void test_reclaim_function(void* node)
{
    (void)node;
}

MOCK_FUNCTION_WITH_CODE(, void, test_item_cleanup_func, void*, context, struct CLDS_HASH_TABLE_ITEM_TAG*, item)
MOCK_FUNCTION_END()

MOCK_FUNCTION_WITH_CODE(, uint64_t, test_compute_hash, void*, key)
    (void)key;
MOCK_FUNCTION_END((uint64_t)key)

MOCK_FUNCTION_WITH_CODE(, void, test_skipped_seq_no_cb, void*, context, int64_t, skipped_seq_no)
MOCK_FUNCTION_END()

static CLDS_CONDITION_CHECK_RESULT g_condition_check_result = CLDS_CONDITION_CHECK_OK;
MOCK_FUNCTION_WITH_CODE(, CLDS_CONDITION_CHECK_RESULT, test_item_condition_check, void*, context, void*, new_key, void*, old_key)
MOCK_FUNCTION_END(g_condition_check_result)

static int test_key_compare_func(void* key_1, void* key_2)
{
    int result;
    if (key_1 < key_2)
    {
        result = -1;
    }
    else if (key_1 > key_2)
    {
        result = 1;
    }
    else
    {
        result = 0;
    }

    return result;
}

typedef struct TEST_ITEM_TAG
{
    int dummy;
} TEST_ITEM;

DECLARE_HASH_TABLE_NODE_TYPE(TEST_ITEM)

BEGIN_TEST_SUITE(TEST_SUITE_NAME_FROM_CMAKE)

TEST_SUITE_INITIALIZE(suite_init)
{
    ASSERT_ARE_EQUAL(int, 0, real_gballoc_hl_init(NULL, NULL));

    ASSERT_ARE_EQUAL(int, 0, umock_c_init(on_umock_c_error), "umock_c_init failed");

    ASSERT_ARE_EQUAL(int, 0, umocktypes_stdint_register_types(), "umocktypes_stdint_register_types failed");
    ASSERT_ARE_EQUAL(int, 0, umocktypes_bool_register_types(), "umocktypes_bool_register_types failed");

    REGISTER_CLDS_ST_HASH_SET_GLOBAL_MOCK_HOOKS();
    REGISTER_CLDS_HAZARD_POINTERS_GLOBAL_MOCK_HOOKS();
    REGISTER_CLDS_SORTED_LIST_GLOBAL_MOCK_HOOKS();

    REGISTER_GBALLOC_HL_GLOBAL_MOCK_HOOK();

    REGISTER_GLOBAL_MOCK_FAIL_RETURN(clds_sorted_list_get_count, CLDS_SORTED_LIST_GET_COUNT_ERROR);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(clds_sorted_list_get_all, CLDS_SORTED_LIST_GET_ALL_ERROR);

    REGISTER_UMOCK_ALIAS_TYPE(RECLAIM_FUNC, void*);
    REGISTER_UMOCK_ALIAS_TYPE(CLDS_HAZARD_POINTERS_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(CLDS_HAZARD_POINTER_RECORD_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(CLDS_SORTED_LIST_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(CLDS_HAZARD_POINTERS_THREAD_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(SORTED_LIST_ITEM_CLEANUP_CB, void*);
    REGISTER_UMOCK_ALIAS_TYPE(SORTED_LIST_GET_ITEM_KEY_CB, void*);
    REGISTER_UMOCK_ALIAS_TYPE(SORTED_LIST_KEY_COMPARE_CB, void*);
    REGISTER_UMOCK_ALIAS_TYPE(CLDS_ST_HASH_SET_COMPUTE_HASH_FUNC, void*);
    REGISTER_UMOCK_ALIAS_TYPE(CLDS_ST_HASH_SET_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(SORTED_LIST_SKIPPED_SEQ_NO_CB, void*);
    REGISTER_UMOCK_ALIAS_TYPE(CLDS_ST_HASH_SET_KEY_COMPARE_FUNC, void*);
    REGISTER_UMOCK_ALIAS_TYPE(CONDITION_CHECK_CB, void*);

    REGISTER_TYPE(CLDS_SORTED_LIST_INSERT_RESULT, CLDS_SORTED_LIST_INSERT_RESULT);
    REGISTER_TYPE(CLDS_SORTED_LIST_DELETE_RESULT, CLDS_SORTED_LIST_DELETE_RESULT);
    REGISTER_TYPE(CLDS_SORTED_LIST_REMOVE_RESULT, CLDS_SORTED_LIST_REMOVE_RESULT);
    REGISTER_TYPE(CLDS_SORTED_LIST_SET_VALUE_RESULT, CLDS_SORTED_LIST_SET_VALUE_RESULT);
    REGISTER_TYPE(CLDS_HASH_TABLE_INSERT_RESULT, CLDS_HASH_TABLE_INSERT_RESULT);
    REGISTER_TYPE(CLDS_HASH_TABLE_DELETE_RESULT, CLDS_HASH_TABLE_DELETE_RESULT);
    REGISTER_TYPE(CLDS_HASH_TABLE_REMOVE_RESULT, CLDS_HASH_TABLE_REMOVE_RESULT);
    REGISTER_TYPE(CLDS_HASH_TABLE_SET_VALUE_RESULT, CLDS_HASH_TABLE_SET_VALUE_RESULT);
    REGISTER_TYPE(CLDS_HASH_TABLE_SNAPSHOT_RESULT, CLDS_HASH_TABLE_SNAPSHOT_RESULT);
    REGISTER_TYPE(CLDS_CONDITION_CHECK_RESULT, CLDS_CONDITION_CHECK_RESULT);

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_negative_tests_deinit();
    umock_c_deinit();

    real_gballoc_hl_deinit();
}

TEST_FUNCTION_INITIALIZE(method_init)
{
    g_condition_check_result = CLDS_CONDITION_CHECK_OK;
    umock_c_reset_all_calls();
}

TEST_FUNCTION_CLEANUP(method_cleanup)
{
}

/* clds_hash_table_create */

/* Tests_SRS_CLDS_HASH_TABLE_01_001: [ clds_hash_table_create shall create a new hash table object and on success it shall return a non-NULL handle to the newly created hash table. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_027: [ The hash table shall maintain a list of arrays of buckets, so that it can be resized as needed. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_058: [ start_sequence_number shall be allowed to be NULL, in which case no sequence number computations shall be performed. ]*/
TEST_FUNCTION(clds_hash_table_create_succeeds)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HASH_TABLE_HANDLE hash_table;
    volatile_atomic int64_t sequence_number = 55;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(malloc(IGNORED_ARG));
    STRICT_EXPECTED_CALL(malloc_flex(IGNORED_ARG, 1, IGNORED_ARG));

    // act
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 1, hazard_pointers, &sequence_number, test_skipped_seq_no_cb, (void*)0x5556);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(hash_table);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_001: [ clds_hash_table_create shall create a new hash table object and on success it shall return a non-NULL handle to the newly created hash table. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_027: [ The hash table shall maintain a list of arrays of buckets, so that it can be resized as needed. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_058: [ start_sequence_number shall be allowed to be NULL, in which case no sequence number computations shall be performed. ]*/
TEST_FUNCTION(clds_hash_table_create_succeeds_with_NULL_skipped_seq_no_cb)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HASH_TABLE_HANDLE hash_table;
    volatile_atomic int64_t sequence_number = 55;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(malloc(IGNORED_ARG));
    STRICT_EXPECTED_CALL(malloc_flex(IGNORED_ARG, 1, IGNORED_ARG));

    // act
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 1, hazard_pointers, &sequence_number, NULL, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(hash_table);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_001: [ clds_hash_table_create shall create a new hash table object and on success it shall return a non-NULL handle to the newly created hash table. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_027: [ The hash table shall maintain a list of arrays of buckets, so that it can be resized as needed. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_058: [ start_sequence_number shall be allowed to be NULL, in which case no sequence number computations shall be performed. ]*/
TEST_FUNCTION(clds_hash_table_create_succeeds_with_NULL_sequence_number_and_skipped_seq_no_cb)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HASH_TABLE_HANDLE hash_table;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(malloc(IGNORED_ARG));
    STRICT_EXPECTED_CALL(malloc_flex(IGNORED_ARG, 1, IGNORED_ARG));

    // act
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 1, hazard_pointers, NULL, NULL, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(hash_table);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_058: [ start_sequence_number shall be allowed to be NULL, in which case no sequence number computations shall be performed. ]*/
TEST_FUNCTION(clds_hash_table_create_with_NULL_sequence_number_and_non_NULL_skipped_seq_no_cb_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HASH_TABLE_HANDLE hash_table;
    umock_c_reset_all_calls();

    // act
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 1, hazard_pointers, NULL, test_skipped_seq_no_cb, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(hash_table);

    // cleanup
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_002: [ If any error happens, clds_hash_table_create shall fail and return NULL. ]*/
TEST_FUNCTION(when_allocating_memory_for_the_hash_table_fails_clds_hash_table_create_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HASH_TABLE_HANDLE hash_table;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(malloc(IGNORED_ARG))
        .SetReturn(NULL);

    // act
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 1, hazard_pointers, NULL, NULL, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(hash_table);

    // cleanup
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_002: [ If any error happens, clds_hash_table_create shall fail and return NULL. ]*/
TEST_FUNCTION(when_allocating_memory_for_the_hash_table_array_fails_clds_hash_table_create_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HASH_TABLE_HANDLE hash_table;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(malloc(IGNORED_ARG));
    STRICT_EXPECTED_CALL(malloc_flex(IGNORED_ARG, 1, IGNORED_ARG))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(free(IGNORED_ARG));

    // act
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 1, hazard_pointers, NULL, NULL, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(hash_table);

    // cleanup
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_003: [ If compute_hash is NULL, clds_hash_table_create shall fail and return NULL. ]*/
TEST_FUNCTION(clds_hash_table_create_with_NULL_compute_hash_function_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HASH_TABLE_HANDLE hash_table;
    umock_c_reset_all_calls();

    // act
    hash_table = clds_hash_table_create(NULL, test_key_compare_func, 1, hazard_pointers, NULL, NULL, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(hash_table);

    // cleanup
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_045: [ If key_compare_func is NULL, clds_hash_table_create shall fail and return NULL. ]*/
TEST_FUNCTION(clds_hash_table_create_with_NULL_key_compare_function_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HASH_TABLE_HANDLE hash_table;
    umock_c_reset_all_calls();

    // act
    hash_table = clds_hash_table_create(test_compute_hash, NULL, 1, hazard_pointers, NULL, NULL, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(hash_table);

    // cleanup
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_004: [ If initial_bucket_size is 0, clds_hash_table_create shall fail and return NULL. ]*/
TEST_FUNCTION(clds_hash_table_create_with_initial_bucket_size_zero_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HASH_TABLE_HANDLE hash_table;
    umock_c_reset_all_calls();

    // act
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 0, hazard_pointers, NULL, NULL, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(hash_table);

    // cleanup
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_005: [ If clds_hazard_pointers is NULL, clds_hash_table_create shall fail and return NULL. ]*/
TEST_FUNCTION(clds_hash_table_create_with_NULL_clds_hazard_pointers_fails)
{
    // arrange

    // act
    CLDS_HASH_TABLE_HANDLE hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 1, NULL, NULL, NULL, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(hash_table);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_001: [ clds_hash_table_create shall create a new hash table object and on success it shall return a non-NULL handle to the newly created hash table. ]*/
TEST_FUNCTION(two_hash_tables_can_be_created_with_the_same_hazard_pointer_instance)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HASH_TABLE_HANDLE hash_table_1;
    CLDS_HASH_TABLE_HANDLE hash_table_2;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(malloc(IGNORED_ARG));
    STRICT_EXPECTED_CALL(malloc_flex(IGNORED_ARG, 1, IGNORED_ARG));

    STRICT_EXPECTED_CALL(malloc(IGNORED_ARG));
    STRICT_EXPECTED_CALL(malloc_flex(IGNORED_ARG, 1, IGNORED_ARG));

    // act
    hash_table_1 = clds_hash_table_create(test_compute_hash, test_key_compare_func, 1, hazard_pointers, NULL, NULL, NULL);
    hash_table_2 = clds_hash_table_create(test_compute_hash, test_key_compare_func, 1, hazard_pointers, NULL, NULL, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(hash_table_1);
    ASSERT_IS_NOT_NULL(hash_table_2);
    ASSERT_ARE_NOT_EQUAL(void_ptr, hash_table_1, hash_table_2);

    // cleanup
    clds_hash_table_destroy(hash_table_1);
    clds_hash_table_destroy(hash_table_2);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_001: [ clds_hash_table_create shall create a new hash table object and on success it shall return a non-NULL handle to the newly created hash table. ]*/
TEST_FUNCTION(two_hash_tables_can_be_created_with_different_hazard_pointer_instances)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers_1 = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers_2 = clds_hazard_pointers_create();
    CLDS_HASH_TABLE_HANDLE hash_table_1;
    CLDS_HASH_TABLE_HANDLE hash_table_2;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(malloc(IGNORED_ARG));
    STRICT_EXPECTED_CALL(malloc_flex(IGNORED_ARG, 1, IGNORED_ARG));

    STRICT_EXPECTED_CALL(malloc(IGNORED_ARG));
    STRICT_EXPECTED_CALL(malloc_flex(IGNORED_ARG, 1, IGNORED_ARG));

    // act
    hash_table_1 = clds_hash_table_create(test_compute_hash, test_key_compare_func, 1, hazard_pointers_1, NULL, NULL, NULL);
    hash_table_2 = clds_hash_table_create(test_compute_hash, test_key_compare_func, 1, hazard_pointers_2, NULL, NULL, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(hash_table_1);
    ASSERT_IS_NOT_NULL(hash_table_2);
    ASSERT_ARE_NOT_EQUAL(void_ptr, hash_table_1, hash_table_2);

    // cleanup
    clds_hash_table_destroy(hash_table_1);
    clds_hash_table_destroy(hash_table_2);
    clds_hazard_pointers_destroy(hazard_pointers_1);
    clds_hazard_pointers_destroy(hazard_pointers_2);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_001: [ clds_hash_table_create shall create a new hash table object and on success it shall return a non-NULL handle to the newly created hash table. ]*/
TEST_FUNCTION(clds_hash_table_create_with_initial_size_2_succeeds)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HASH_TABLE_HANDLE hash_table;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(malloc(IGNORED_ARG));
    STRICT_EXPECTED_CALL(malloc_flex(IGNORED_ARG, 2, IGNORED_ARG));

    // act
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, NULL, NULL, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(hash_table);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_057: [ start_sequence_number shall be used as the sequence number variable that shall be incremented at every operation that is done on the hash table. ]*/
TEST_FUNCTION(clds_hash_table_create_with_non_NULL_start_sequence_number_succeeds)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HASH_TABLE_HANDLE hash_table;
    volatile_atomic int64_t sequence_number;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(malloc(IGNORED_ARG));
    STRICT_EXPECTED_CALL(malloc_flex(IGNORED_ARG, 1, IGNORED_ARG));

    // act
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 1, hazard_pointers, &sequence_number, NULL, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(hash_table);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* clds_hash_table_destroy */

/* Tests_SRS_CLDS_HASH_TABLE_01_006: [ clds_hash_table_destroy shall free all resources associated with the hash table instance. ]*/
TEST_FUNCTION(clds_hash_table_destroy_frees_the_hash_table_resources)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HASH_TABLE_HANDLE hash_table;
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, NULL, NULL, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(free(IGNORED_ARG));
    STRICT_EXPECTED_CALL(free(IGNORED_ARG));

    // act
    clds_hash_table_destroy(hash_table);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_007: [ If clds_hash_table is NULL, clds_hash_table_destroy shall return. ]*/
TEST_FUNCTION(clds_hash_table_destroy_with_NULL_hash_table_returns)
{
    // arrange

    // act
    clds_hash_table_destroy(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* clds_hash_table_insert */

/* Tests_SRS_CLDS_HASH_TABLE_01_009: [ On success clds_hash_table_insert shall return CLDS_HASH_TABLE_INSERT_OK. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_018: [ clds_hash_table_insert shall obtain the bucket index to be used by calling compute_hash and passing to it the key value. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_019: [ If no sorted list exists at the determined bucket index then a new list shall be created. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_020: [ A new sorted list item shall be created by calling clds_sorted_list_node_create. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_021: [ The new sorted list node shall be inserted in the sorted list at the identified bucket by calling clds_sorted_list_insert. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_038: [ clds_hash_table_insert shall hash the key by calling the compute_hash function passed to clds_hash_table_create. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_071: [ When a new list is created, the start sequence number passed to clds_hash_tabel_create shall be passed as the start_sequence_number argument. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_059: [ For each insert the order of the operation shall be computed by passing sequence_number to clds_sorted_list_insert. ]*/
TEST_FUNCTION(clds_hash_table_insert_inserts_one_key_value_pair)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_SORTED_LIST_HANDLE linked_list;
    CLDS_HASH_TABLE_INSERT_RESULT result;
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, NULL, NULL, NULL);
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x1));
    STRICT_EXPECTED_CALL(clds_sorted_list_create(hazard_pointers, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, NULL, IGNORED_ARG, IGNORED_ARG))
        .CaptureReturn(&linked_list);
    STRICT_EXPECTED_CALL(clds_sorted_list_insert(IGNORED_ARG, IGNORED_ARG, (CLDS_SORTED_LIST_ITEM*)item, NULL))
        .ValidateArgumentValue_clds_sorted_list(&linked_list);

    // act
    result = clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_INSERT_RESULT, CLDS_HASH_TABLE_INSERT_OK, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_071: [ When a new list is created, the start sequence number passed to clds_hash_tabel_create shall be passed as the start_sequence_number argument. ]*/
TEST_FUNCTION(clds_hash_table_insert_inserts_one_key_value_pair_with_non_NULL_start_sequence_no)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_SORTED_LIST_HANDLE linked_list;
    CLDS_HASH_TABLE_INSERT_RESULT result;
    volatile_atomic int64_t sequence_number = 42;
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, &sequence_number, NULL, NULL);
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x1));
    STRICT_EXPECTED_CALL(clds_sorted_list_create(hazard_pointers, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, &sequence_number, IGNORED_ARG, IGNORED_ARG))
        .CaptureReturn(&linked_list);
    STRICT_EXPECTED_CALL(clds_sorted_list_insert(IGNORED_ARG, IGNORED_ARG, (CLDS_SORTED_LIST_ITEM*)item, NULL))
        .ValidateArgumentValue_clds_sorted_list(&linked_list);

    // act
    result = clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_INSERT_RESULT, CLDS_HASH_TABLE_INSERT_OK, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_010: [ If clds_hash_table is NULL, clds_hash_table_insert shall fail and return CLDS_HASH_TABLE_INSERT_ERROR. ]*/
TEST_FUNCTION(clds_hash_table_insert_with_NULL_hash_table_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_INSERT_RESULT result;
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    umock_c_reset_all_calls();

    // act
    result = clds_hash_table_insert(NULL, hazard_pointers_thread, (void*)0x1, item, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_INSERT_RESULT, CLDS_HASH_TABLE_INSERT_ERROR, result);

    // cleanup
    CLDS_HASH_TABLE_NODE_RELEASE(TEST_ITEM, item);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_012: [ If clds_hazard_pointers_thread is NULL, clds_hash_table_insert shall fail and return CLDS_HASH_TABLE_INSERT_ERROR. ]*/
TEST_FUNCTION(clds_hash_table_insert_with_NULL_clds_hazard_pointers_thread_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_HASH_TABLE_INSERT_RESULT result;
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, NULL, NULL, NULL);
    umock_c_reset_all_calls();

    // act
    result = clds_hash_table_insert(hash_table, NULL, (void*)0x1, item, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_INSERT_RESULT, CLDS_HASH_TABLE_INSERT_ERROR, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    CLDS_HASH_TABLE_NODE_RELEASE(TEST_ITEM, item);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_011: [ If key is NULL, clds_hash_table_insert shall fail and return CLDS_HASH_TABLE_INSERT_ERROR. ]*/
TEST_FUNCTION(clds_hash_table_insert_with_NULL_key_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_INSERT_RESULT result;
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, NULL, NULL, NULL);
    umock_c_reset_all_calls();

    // act
    result = clds_hash_table_insert(hash_table, hazard_pointers_thread, NULL, item, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_INSERT_RESULT, CLDS_HASH_TABLE_INSERT_ERROR, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    CLDS_HASH_TABLE_NODE_RELEASE(TEST_ITEM, item);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_022: [ If any error is encountered while inserting the key/value pair, clds_hash_table_insert shall fail and return CLDS_HASH_TABLE_INSERT_ERROR. ]*/
TEST_FUNCTION(when_creating_the_singly_linked_list_fails_clds_hash_table_insert_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_HASH_TABLE_INSERT_RESULT result;
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, NULL, NULL, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x1));
    STRICT_EXPECTED_CALL(clds_sorted_list_create(hazard_pointers, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG))
        .SetReturn(NULL);

    // act
    result = clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_INSERT_RESULT, CLDS_HASH_TABLE_INSERT_ERROR, result);

    // cleanup
    CLDS_HASH_TABLE_NODE_RELEASE(TEST_ITEM, item);
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_022: [ If any error is encountered while inserting the key/value pair, clds_hash_table_insert shall fail and return CLDS_HASH_TABLE_INSERT_ERROR. ]*/
TEST_FUNCTION(when_inserting_the_singly_linked_list_item_fails_clds_hash_table_insert_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_HASH_TABLE_INSERT_RESULT result;
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, NULL, NULL, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x1));
    STRICT_EXPECTED_CALL(clds_sorted_list_create(hazard_pointers, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG));
    STRICT_EXPECTED_CALL(clds_sorted_list_insert(IGNORED_ARG, IGNORED_ARG, (CLDS_SORTED_LIST_ITEM*)item, NULL))
        .SetReturn(CLDS_SORTED_LIST_INSERT_ERROR);

    // act
    result = clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_INSERT_RESULT, CLDS_HASH_TABLE_INSERT_ERROR, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    CLDS_HASH_TABLE_NODE_RELEASE(TEST_ITEM, item);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_009: [ On success clds_hash_table_insert shall return CLDS_HASH_TABLE_INSERT_OK. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_018: [ clds_hash_table_insert shall obtain the bucket index to be used by calling compute_hash and passing to it the key value. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_020: [ A new sorted list item shall be created by calling clds_sorted_list_node_create. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_021: [ The new sorted list node shall be inserted in the sorted list at the identified bucket by calling clds_sorted_list_insert. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_038: [ clds_hash_table_insert shall hash the key by calling the compute_hash function passed to clds_hash_table_create. ]*/
TEST_FUNCTION(clds_hash_table_insert_with_2nd_key_on_the_same_bucket_does_not_create_another_list)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_HASH_TABLE_INSERT_RESULT result;
    uint64_t first_hash;
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, NULL, NULL, NULL);
    CLDS_HASH_TABLE_ITEM* item_1 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* item_2 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x1))
        .CaptureReturn(&first_hash);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item_1, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(clds_hazard_pointers_acquire(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_release(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x2))
        .SetReturn(first_hash);
    STRICT_EXPECTED_CALL(clds_sorted_list_insert(IGNORED_ARG, IGNORED_ARG, (CLDS_SORTED_LIST_ITEM*)item_2, NULL));

    // act
    result = clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x2, item_2, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_INSERT_RESULT, CLDS_HASH_TABLE_INSERT_OK, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_009: [ On success clds_hash_table_insert shall return CLDS_HASH_TABLE_INSERT_OK. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_018: [ clds_hash_table_insert shall obtain the bucket index to be used by calling compute_hash and passing to it the key value. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_020: [ A new sorted list item shall be created by calling clds_sorted_list_node_create. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_021: [ The new sorted list node shall be inserted in the sorted list at the identified bucket by calling clds_sorted_list_insert. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_038: [ clds_hash_table_insert shall hash the key by calling the compute_hash function passed to clds_hash_table_create. ]*/
TEST_FUNCTION(clds_hash_table_insert_with_2nd_key_on_a_different_bucket_creates_another_list)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_SORTED_LIST_HANDLE linked_list;
    CLDS_HASH_TABLE_INSERT_RESULT result;
    CLDS_HASH_TABLE_ITEM* item_1 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* item_2 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 4, hazard_pointers, NULL, NULL, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item_1, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x2));
    STRICT_EXPECTED_CALL(clds_sorted_list_create(hazard_pointers, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG))
        .CaptureReturn(&linked_list);
    STRICT_EXPECTED_CALL(clds_sorted_list_insert(IGNORED_ARG, IGNORED_ARG, (CLDS_SORTED_LIST_ITEM*)item_2, NULL))
        .ValidateArgumentValue_clds_sorted_list(&linked_list);

    // act
    result = clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x2, item_2, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_INSERT_RESULT, CLDS_HASH_TABLE_INSERT_OK, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_030: [ If the number of items in the list reaches the number of buckets, the number of buckets shall be doubled. ]*/
TEST_FUNCTION(clds_hash_table_insert_when_the_number_of_items_reaches_number_of_buckets_allocates_another_bucket_array)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_SORTED_LIST_HANDLE linked_list;
    CLDS_HASH_TABLE_INSERT_RESULT result;
    CLDS_HASH_TABLE_ITEM* item_1 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, NULL, NULL);
    CLDS_HASH_TABLE_ITEM* item_2 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, NULL, NULL);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 1, hazard_pointers, NULL, NULL, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item_1, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(clds_hazard_pointers_acquire(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_release(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(malloc_flex(IGNORED_ARG, IGNORED_ARG, IGNORED_ARG));
    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x2));
    STRICT_EXPECTED_CALL(clds_sorted_list_find_key(IGNORED_ARG, hazard_pointers_thread, (void*)0x2));
    STRICT_EXPECTED_CALL(clds_sorted_list_create(hazard_pointers, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG))
        .CaptureReturn(&linked_list);
    STRICT_EXPECTED_CALL(clds_sorted_list_insert(IGNORED_ARG, hazard_pointers_thread, (CLDS_SORTED_LIST_ITEM*)item_2, NULL))
        .ValidateArgumentValue_clds_sorted_list(&linked_list);

    // act
    result = clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x2, item_2, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_INSERT_RESULT, CLDS_HASH_TABLE_INSERT_OK, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_046: [ If the key already exists in the hash table, clds_hash_table_insert shall fail and return CLDS_HASH_TABLE_INSERT_ALREADY_EXISTS. ]*/
TEST_FUNCTION(clds_hash_table_insert_with_the_same_key_2_times_returns_KEY_ALREADY_EXISTS)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_HASH_TABLE_INSERT_RESULT result;
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, NULL, NULL, NULL);
    CLDS_HASH_TABLE_ITEM* item_1 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* item_2 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    umock_c_reset_all_calls();
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item_1, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(clds_hazard_pointers_acquire(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_release(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x1));
    STRICT_EXPECTED_CALL(clds_sorted_list_insert(IGNORED_ARG, IGNORED_ARG, (CLDS_SORTED_LIST_ITEM*)item_2, NULL));

    // act
    result = clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item_2, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_INSERT_RESULT, CLDS_HASH_TABLE_INSERT_KEY_ALREADY_EXISTS, result);

    // cleanup
    CLDS_HASH_TABLE_NODE_RELEASE(TEST_ITEM, item_2);
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_059: [ For each insert the order of the operation shall be computed by passing sequence_number to clds_sorted_list_insert. ]*/
TEST_FUNCTION(clds_hash_table_insert_passes_the_sequence_number_to_the_sorted_list_insert)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_SORTED_LIST_HANDLE linked_list;
    CLDS_HASH_TABLE_INSERT_RESULT result;
    volatile_atomic int64_t sequence_number = 42;
    int64_t insert_seq_no = 0;
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, &sequence_number, NULL, NULL);
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x1));
    STRICT_EXPECTED_CALL(clds_sorted_list_create(hazard_pointers, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, &sequence_number, IGNORED_ARG, IGNORED_ARG))
        .CaptureReturn(&linked_list);
    STRICT_EXPECTED_CALL(clds_sorted_list_insert(IGNORED_ARG, IGNORED_ARG, (CLDS_SORTED_LIST_ITEM*)item, &insert_seq_no))
        .ValidateArgumentValue_clds_sorted_list(&linked_list);

    // act
    result = clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item, &insert_seq_no);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_INSERT_RESULT, CLDS_HASH_TABLE_INSERT_OK, result);
    ASSERT_ARE_EQUAL(int64_t, 43, insert_seq_no);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_062: [ If the sequence_number argument is non-NULL, but no start sequence number was specified in clds_hash_table_create, clds_hash_table_insert shall fail and return CLDS_HASH_TABLE_INSERT_ERROR. ]*/
TEST_FUNCTION(clds_hash_table_insert_with_non_NULL_sequence_no_but_NULL_start_sequence_no_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_HASH_TABLE_INSERT_RESULT result;
    int64_t insert_seq_no = 0;
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, NULL, NULL, NULL);
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    umock_c_reset_all_calls();

    // act
    result = clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item, &insert_seq_no);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_INSERT_RESULT, CLDS_HASH_TABLE_INSERT_ERROR, result);

    // cleanup
    CLDS_HASH_TABLE_NODE_RELEASE(TEST_ITEM, item);
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* clds_hash_table_delete */

/* Tests_SRS_CLDS_HASH_TABLE_01_014: [ On success clds_hash_table_delete shall return CLDS_HASH_TABLE_DELETE_OK. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_039: [ clds_hash_table_delete shall hash the key by calling the compute_hash function passed to clds_hash_table_create. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_063: [ For each delete the order of the operation shall be computed by passing sequence_number to clds_sorted_list_delete_key. ]*/
TEST_FUNCTION(clds_hash_table_delete_deletes_the_key)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_HASH_TABLE_DELETE_RESULT result;
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, NULL, NULL, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(clds_hazard_pointers_acquire(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_release(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_reclaim(IGNORED_ARG, IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x1));
    STRICT_EXPECTED_CALL(clds_sorted_list_delete_key(IGNORED_ARG, hazard_pointers_thread, (void*)0x1, NULL));
    STRICT_EXPECTED_CALL(test_item_cleanup_func((void*)0x4242, IGNORED_ARG));

    // act
    result = clds_hash_table_delete(hash_table, hazard_pointers_thread, (void*)0x1, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_DELETE_RESULT, CLDS_HASH_TABLE_DELETE_OK, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_015: [ If clds_hash_table is NULL, clds_hash_table_delete shall fail and return CLDS_HASH_TABLE_DELETE_ERROR. ]*/
TEST_FUNCTION(clds_hash_table_delete_with_NULL_hash_table_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    int result;
    umock_c_reset_all_calls();

    // act
    result = clds_hash_table_delete(NULL, hazard_pointers_thread, (void*)0x1, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_017: [ If clds_hazard_pointers_thread is NULL, clds_hash_table_delete shall fail and return CLDS_HASH_TABLE_DELETE_ERROR. ]*/
TEST_FUNCTION(clds_hash_table_delete_with_NULL_clds_hazard_pointers_thread_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    int result;
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, NULL, NULL, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL);
    umock_c_reset_all_calls();

    // act
    result = clds_hash_table_delete(hash_table, NULL, (void*)0x1, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_016: [ If key is NULL, clds_hash_table_delete shall fail and return CLDS_HASH_TABLE_DELETE_ERROR. ]*/
TEST_FUNCTION(clds_hash_table_delete_with_NULL_key_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    int result;
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, NULL, NULL, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL);
    umock_c_reset_all_calls();

    // act
    result = clds_hash_table_delete(hash_table, hazard_pointers_thread, NULL, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_023: [ If the desired key is not found in the hash table (not found in any of the arrays of buckets), clds_hash_table_delete shall return CLDS_HASH_TABLE_DELETE_NOT_FOUND. ]*/
TEST_FUNCTION(clds_hash_table_delete_with_a_key_that_is_not_in_the_hash_table_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    int result;
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, NULL, NULL, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x1));

    // act
    result = clds_hash_table_delete(hash_table, hazard_pointers_thread, (void*)0x1, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_023: [ If the desired key is not found in the hash table (not found in any of the arrays of buckets), clds_hash_table_delete shall return CLDS_HASH_TABLE_DELETE_NOT_FOUND. ]*/
TEST_FUNCTION(clds_hash_table_delete_with_a_key_that_is_already_deleted_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    int result;
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, NULL, NULL, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL);
    (void)clds_hash_table_delete(hash_table, hazard_pointers_thread, (void*)0x1, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x1));

    // act
    result = clds_hash_table_delete(hash_table, hazard_pointers_thread, (void*)0x1, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_024: [ If a bucket is identified and the delete of the item from the underlying list fails, clds_hash_table_delete shall fail and return CLDS_HASH_TABLE_DELETE_ERROR. ]*/
TEST_FUNCTION(when_the_underlying_delete_from_list_fails_clds_hash_table_delete_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_HASH_TABLE_DELETE_RESULT result;
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, NULL, NULL, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x1));
    STRICT_EXPECTED_CALL(clds_sorted_list_delete_key(IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, NULL))
        .SetReturn(CLDS_SORTED_LIST_DELETE_ERROR);

    // act
    result = clds_hash_table_delete(hash_table, hazard_pointers_thread, (void*)0x1, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_DELETE_RESULT, CLDS_HASH_TABLE_DELETE_ERROR, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_101: [ Otherwise, key shall be looked up in each of the arrays of buckets starting with the first. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_025: [ If the element to be deleted is not found in an array of buckets, then it shall be looked up in the next available array of buckets. ] */
TEST_FUNCTION(delete_looks_in_2_buckets)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_HASH_TABLE_DELETE_RESULT result;
    CLDS_HASH_TABLE_ITEM* item_1 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* item_2 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 1, hazard_pointers, NULL, NULL, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item_1, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x2, item_2, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(clds_hazard_pointers_acquire(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_release(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_reclaim(IGNORED_ARG, IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x1));
    STRICT_EXPECTED_CALL(clds_sorted_list_delete_key(IGNORED_ARG, IGNORED_ARG, (void*)0x1, NULL));
    STRICT_EXPECTED_CALL(test_item_cleanup_func((void*)0x4242, IGNORED_ARG));

    // act
    result = clds_hash_table_delete(hash_table, hazard_pointers_thread, (void*)0x1, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_DELETE_RESULT, CLDS_HASH_TABLE_DELETE_OK, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_101: [ Otherwise, key shall be looked up in each of the arrays of buckets starting with the first. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_025: [ If the element to be deleted is not found in an array of buckets, then it shall be looked up in the next available array of buckets. ] */
TEST_FUNCTION(delete_looks_in_3_buckets)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_HASH_TABLE_DELETE_RESULT result;
    // 1st bucket array has 1 bucket
    CLDS_HASH_TABLE_ITEM* item_1 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    // 2nd bucket array has 2 buckets
    CLDS_HASH_TABLE_ITEM* item_2 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* item_3 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    // 3rd bucket array has 4 buckets
    CLDS_HASH_TABLE_ITEM* item_4 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 1, hazard_pointers, NULL, NULL, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item_1, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x2, item_2, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x3, item_3, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x4, item_4, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(clds_hazard_pointers_acquire(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_release(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_reclaim(IGNORED_ARG, IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x1));
    STRICT_EXPECTED_CALL(clds_sorted_list_delete_key(IGNORED_ARG, IGNORED_ARG, (void*)0x1, NULL));
    STRICT_EXPECTED_CALL(clds_sorted_list_delete_key(IGNORED_ARG, IGNORED_ARG, (void*)0x1, NULL));
    STRICT_EXPECTED_CALL(test_item_cleanup_func((void*)0x4242, IGNORED_ARG));

    // act
    result = clds_hash_table_delete(hash_table, hazard_pointers_thread, (void*)0x1, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_DELETE_RESULT, CLDS_HASH_TABLE_DELETE_OK, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_023: [ If the desired key is not found in the hash table (not found in any of the arrays of buckets), clds_hash_table_delete shall return CLDS_HASH_TABLE_DELETE_NOT_FOUND. ]*/
TEST_FUNCTION(when_item_is_not_found_in_any_bucket_delete_returns_NOT_FOUND)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_HASH_TABLE_DELETE_RESULT result;
    CLDS_HASH_TABLE_ITEM* item_1 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* item_2 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 1, hazard_pointers, NULL, NULL, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item_1, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x2, item_2, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(clds_hazard_pointers_acquire(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_release(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_reclaim(IGNORED_ARG, IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x3));
    STRICT_EXPECTED_CALL(clds_sorted_list_delete_key(IGNORED_ARG, IGNORED_ARG, (void*)0x3, NULL));

    // act
    result = clds_hash_table_delete(hash_table, hazard_pointers_thread, (void*)0x3, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_DELETE_RESULT, CLDS_HASH_TABLE_DELETE_NOT_FOUND, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_063: [ For each delete the order of the operation shall be computed by passing sequence_number to clds_sorted_list_delete_key. ]*/
TEST_FUNCTION(clds_hash_table_delete_deletes_the_key_and_stamps_the_sequence_no)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    volatile_atomic int64_t sequence_number = 42;
    int64_t delete_seq_no = 0;
    CLDS_HASH_TABLE_DELETE_RESULT result;
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, &sequence_number, NULL, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(clds_hazard_pointers_acquire(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_release(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_reclaim(IGNORED_ARG, IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x1));
    STRICT_EXPECTED_CALL(clds_sorted_list_delete_key(IGNORED_ARG, hazard_pointers_thread, (void*)0x1, &delete_seq_no));
    STRICT_EXPECTED_CALL(test_item_cleanup_func((void*)0x4242, IGNORED_ARG));

    // act
    result = clds_hash_table_delete(hash_table, hazard_pointers_thread, (void*)0x1, &delete_seq_no);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_DELETE_RESULT, CLDS_HASH_TABLE_DELETE_OK, result);
    ASSERT_ARE_EQUAL(int64_t, 44, delete_seq_no);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_066: [ If the sequence_number argument is non-NULL, but no start sequence number was specified in clds_hash_table_create, clds_hash_table_delete shall fail and return CLDS_HASH_TABLE_DELETE_ERROR. ]*/
TEST_FUNCTION(clds_hash_table_delete_with_non_NULL_sequence_no_but_NULL_start_sequence_number_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    int64_t delete_seq_no = 0;
    CLDS_HASH_TABLE_DELETE_RESULT result;
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, NULL, NULL, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL);
    umock_c_reset_all_calls();

    // act
    result = clds_hash_table_delete(hash_table, hazard_pointers_thread, (void*)0x1, &delete_seq_no);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_DELETE_RESULT, CLDS_HASH_TABLE_DELETE_ERROR, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* clds_hash_table_delete_key_value */

/* Tests_SRS_CLDS_HASH_TABLE_42_002: [ On success clds_hash_table_delete_key_value shall return CLDS_HASH_TABLE_DELETE_OK. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_001: [ clds_hash_table_delete_key_value shall hash the key by calling the compute_hash function passed to clds_hash_table_create. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_011: [ For each delete the order of the operation shall be computed by passing sequence_number to clds_sorted_list_delete_item. ]*/
TEST_FUNCTION(clds_hash_table_delete_key_value_deletes_the_key)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_HASH_TABLE_DELETE_RESULT result;
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, NULL, NULL, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(clds_hazard_pointers_acquire(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_release(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_reclaim(IGNORED_ARG, IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x1));
    STRICT_EXPECTED_CALL(clds_sorted_list_delete_item(IGNORED_ARG, hazard_pointers_thread, (CLDS_SORTED_LIST_ITEM*)item, NULL));
    STRICT_EXPECTED_CALL(test_item_cleanup_func((void*)0x4242, IGNORED_ARG));

    // act
    result = clds_hash_table_delete_key_value(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_DELETE_RESULT, CLDS_HASH_TABLE_DELETE_OK, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_42_003: [ If clds_hash_table is NULL, clds_hash_table_delete_key_value shall fail and return CLDS_HASH_TABLE_DELETE_ERROR. ]*/
TEST_FUNCTION(clds_hash_table_delete_key_value_with_NULL_hash_table_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    int result;
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    umock_c_reset_all_calls();

    // act
    result = clds_hash_table_delete_key_value(NULL, hazard_pointers_thread, (void*)0x1, item, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    CLDS_HASH_TABLE_NODE_RELEASE(TEST_ITEM, item);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_42_004: [ If clds_hazard_pointers_thread is NULL, clds_hash_table_delete_key_value shall fail and return CLDS_HASH_TABLE_DELETE_ERROR. ]*/
TEST_FUNCTION(clds_hash_table_delete_key_value_with_NULL_clds_hazard_pointers_thread_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    int result;
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, NULL, NULL, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL);
    umock_c_reset_all_calls();

    // act
    result = clds_hash_table_delete_key_value(hash_table, NULL, (void*)0x1, item, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_42_005: [ If key is NULL, clds_hash_table_delete_key_value shall fail and return CLDS_HASH_TABLE_DELETE_ERROR. ]*/
TEST_FUNCTION(clds_hash_table_delete_key_value_with_NULL_key_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    int result;
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, NULL, NULL, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL);
    umock_c_reset_all_calls();

    // act
    result = clds_hash_table_delete_key_value(hash_table, hazard_pointers_thread, NULL, item, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_42_006: [ If value is NULL, clds_hash_table_delete_key_value shall fail and return CLDS_HASH_TABLE_DELETE_ERROR. ]*/
TEST_FUNCTION(clds_hash_table_delete_key_value_with_NULL_value_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    int result;
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, NULL, NULL, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL);
    umock_c_reset_all_calls();

    // act
    result = clds_hash_table_delete_key_value(hash_table, hazard_pointers_thread, (void*)0x1, NULL, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_42_008: [ If the desired key is not found in the hash table (not found in any of the arrays of buckets), clds_hash_table_delete_key_value shall return CLDS_HASH_TABLE_DELETE_NOT_FOUND. ]*/
TEST_FUNCTION(clds_hash_table_delete_key_value_with_a_key_that_is_not_in_the_hash_table_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    int result;
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, NULL, NULL, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x1));

    // act
    result = clds_hash_table_delete_key_value(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    CLDS_HASH_TABLE_NODE_RELEASE(TEST_ITEM, item);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_42_008: [ If the desired key is not found in the hash table (not found in any of the arrays of buckets), clds_hash_table_delete_key_value shall return CLDS_HASH_TABLE_DELETE_NOT_FOUND. ]*/
TEST_FUNCTION(clds_hash_table_delete_key_value_with_a_key_that_is_already_deleted_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    int result;
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, NULL, NULL, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL);
    (void)clds_hash_table_delete(hash_table, hazard_pointers_thread, (void*)0x1, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x1));

    // act
    result = clds_hash_table_delete_key_value(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_42_009: [ If a bucket is identified and the delete of the item from the underlying list fails, clds_hash_table_delete_key_value shall fail and return CLDS_HASH_TABLE_DELETE_ERROR. ]*/
TEST_FUNCTION(when_the_underlying_delete_from_list_fails_clds_hash_table_delete_key_value_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_HASH_TABLE_DELETE_RESULT result;
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, NULL, NULL, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x1));
    STRICT_EXPECTED_CALL(clds_sorted_list_delete_item(IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, NULL))
        .SetReturn(CLDS_SORTED_LIST_DELETE_ERROR);

    // act
    result = clds_hash_table_delete_key_value(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_DELETE_RESULT, CLDS_HASH_TABLE_DELETE_ERROR, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}


/* Tests_SRS_CLDS_HASH_TABLE_42_007: [ Otherwise, key shall be looked up in each of the arrays of buckets starting with the first. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_010: [ If the element to be deleted is not found in an array of buckets, then clds_hash_table_delete_key_value shall look in the next available array of buckets. ]*/
TEST_FUNCTION(clds_hash_table_delete_key_value_looks_in_2_buckets)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_HASH_TABLE_DELETE_RESULT result;
    CLDS_HASH_TABLE_ITEM* item_1 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* item_2 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 1, hazard_pointers, NULL, NULL, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item_1, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x2, item_2, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(clds_hazard_pointers_acquire(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_release(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_reclaim(IGNORED_ARG, IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x1));
    STRICT_EXPECTED_CALL(clds_sorted_list_delete_item(IGNORED_ARG, IGNORED_ARG, (CLDS_SORTED_LIST_ITEM*)item_1, NULL));
    STRICT_EXPECTED_CALL(test_item_cleanup_func((void*)0x4242, IGNORED_ARG));

    // act
    result = clds_hash_table_delete_key_value(hash_table, hazard_pointers_thread, (void*)0x1, item_1, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_DELETE_RESULT, CLDS_HASH_TABLE_DELETE_OK, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_42_007: [ Otherwise, key shall be looked up in each of the arrays of buckets starting with the first. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_010: [ If the element to be deleted is not found in an array of buckets, then clds_hash_table_delete_key_value shall look in the next available array of buckets. ]*/
TEST_FUNCTION(clds_hash_table_delete_key_value_looks_in_3_buckets)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_HASH_TABLE_DELETE_RESULT result;
    // 1st bucket array has 1 bucket
    CLDS_HASH_TABLE_ITEM* item_1 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    // 2nd bucket array has 2 buckets
    CLDS_HASH_TABLE_ITEM* item_2 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* item_3 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    // 3rd bucket array has 4 buckets
    CLDS_HASH_TABLE_ITEM* item_4 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 1, hazard_pointers, NULL, NULL, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item_1, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x2, item_2, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x3, item_3, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x4, item_4, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(clds_hazard_pointers_acquire(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_release(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_reclaim(IGNORED_ARG, IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x1));
    STRICT_EXPECTED_CALL(clds_sorted_list_delete_item(IGNORED_ARG, IGNORED_ARG, (CLDS_SORTED_LIST_ITEM*)item_1, NULL));
    STRICT_EXPECTED_CALL(clds_sorted_list_delete_item(IGNORED_ARG, IGNORED_ARG, (CLDS_SORTED_LIST_ITEM*)item_1, NULL));
    STRICT_EXPECTED_CALL(test_item_cleanup_func((void*)0x4242, IGNORED_ARG));

    // act
    result = clds_hash_table_delete_key_value(hash_table, hazard_pointers_thread, (void*)0x1, item_1, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_DELETE_RESULT, CLDS_HASH_TABLE_DELETE_OK, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_42_008: [ If the desired key is not found in the hash table (not found in any of the arrays of buckets), clds_hash_table_delete_key_value shall return CLDS_HASH_TABLE_DELETE_NOT_FOUND. ]*/
TEST_FUNCTION(when_item_is_not_found_in_any_bucket_clds_hash_table_delete_key_value_returns_NOT_FOUND)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_HASH_TABLE_DELETE_RESULT result;
    CLDS_HASH_TABLE_ITEM* item_1 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* item_2 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* item_3 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 1, hazard_pointers, NULL, NULL, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item_1, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x2, item_2, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(clds_hazard_pointers_acquire(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_release(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_reclaim(IGNORED_ARG, IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x3));
    STRICT_EXPECTED_CALL(clds_sorted_list_delete_item(IGNORED_ARG, IGNORED_ARG, (CLDS_SORTED_LIST_ITEM*)item_3, NULL));

    // act
    result = clds_hash_table_delete_key_value(hash_table, hazard_pointers_thread, (void*)0x3, item_3, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_DELETE_RESULT, CLDS_HASH_TABLE_DELETE_NOT_FOUND, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    CLDS_HASH_TABLE_NODE_RELEASE(TEST_ITEM, item_3);
    clds_hazard_pointers_destroy(hazard_pointers);
}


/* Tests_SRS_CLDS_HASH_TABLE_42_008: [ If the desired key is not found in the hash table (not found in any of the arrays of buckets), clds_hash_table_delete_key_value shall return CLDS_HASH_TABLE_DELETE_NOT_FOUND. ]*/
TEST_FUNCTION(when_item_is_not_found_in_any_bucket_clds_hash_table_delete_key_value_returns_NOT_FOUND_even_when_key_is_found_but_different_value)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_HASH_TABLE_DELETE_RESULT result;
    CLDS_HASH_TABLE_ITEM* item_1 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* item_2 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* item_3 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 1, hazard_pointers, NULL, NULL, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item_1, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x2, item_2, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(clds_hazard_pointers_acquire(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_release(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_reclaim(IGNORED_ARG, IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x1));
    STRICT_EXPECTED_CALL(clds_sorted_list_delete_item(IGNORED_ARG, IGNORED_ARG, (CLDS_SORTED_LIST_ITEM*)item_3, NULL));

    // act
    result = clds_hash_table_delete_key_value(hash_table, hazard_pointers_thread, (void*)0x1, item_3, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_DELETE_RESULT, CLDS_HASH_TABLE_DELETE_NOT_FOUND, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    CLDS_HASH_TABLE_NODE_RELEASE(TEST_ITEM, item_3);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_42_011: [ For each delete the order of the operation shall be computed by passing sequence_number to clds_sorted_list_delete_item. ]*/
TEST_FUNCTION(clds_hash_table_delete_key_value_deletes_the_key_and_stamps_the_sequence_no)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    volatile_atomic int64_t sequence_number = 42;
    int64_t delete_seq_no = 0;
    CLDS_HASH_TABLE_DELETE_RESULT result;
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, &sequence_number, NULL, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(clds_hazard_pointers_acquire(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_release(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_reclaim(IGNORED_ARG, IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x1));
    STRICT_EXPECTED_CALL(clds_sorted_list_delete_item(IGNORED_ARG, hazard_pointers_thread, (CLDS_SORTED_LIST_ITEM*)item, &delete_seq_no));
    STRICT_EXPECTED_CALL(test_item_cleanup_func((void*)0x4242, IGNORED_ARG));

    // act
    result = clds_hash_table_delete_key_value(hash_table, hazard_pointers_thread, (void*)0x1, item, &delete_seq_no);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_DELETE_RESULT, CLDS_HASH_TABLE_DELETE_OK, result);
    ASSERT_ARE_EQUAL(int64_t, 44, delete_seq_no);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_42_012: [ If the sequence_number argument is non-NULL, but no start sequence number was specified in clds_hash_table_create, clds_hash_table_delete_key_value shall fail and return CLDS_HASH_TABLE_DELETE_ERROR. ]*/
TEST_FUNCTION(clds_hash_table_delete_key_value_with_non_NULL_sequence_no_but_NULL_start_sequence_number_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    int64_t delete_seq_no = 0;
    CLDS_HASH_TABLE_DELETE_RESULT result;
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, NULL, NULL, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL);
    umock_c_reset_all_calls();

    // act
    result = clds_hash_table_delete_key_value(hash_table, hazard_pointers_thread, (void*)0x1, item, &delete_seq_no);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_DELETE_RESULT, CLDS_HASH_TABLE_DELETE_ERROR, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* clds_hash_table_remove */

/* Tests_SRS_CLDS_HASH_TABLE_01_047: [ clds_hash_table_remove shall remove a key from the hash table and return a pointer to the item to the user. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_049: [ On success clds_hash_table_remove shall return CLDS_HASH_TABLE_REMOVE_OK. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_048: [ clds_hash_table_remove shall hash the key by calling the compute_hash function passed to clds_hash_table_create. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_067: [ For each remove the order of the operation shall be computed by passing sequence_number to clds_sorted_list_remove_key. ]*/
TEST_FUNCTION(clds_hash_table_remove_removes_the_key_from_the_list)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_HASH_TABLE_REMOVE_RESULT result;
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* removed_item;
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, NULL, NULL, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(clds_hazard_pointers_acquire(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_release(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_reclaim(IGNORED_ARG, IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x1));
    STRICT_EXPECTED_CALL(clds_sorted_list_remove_key(IGNORED_ARG, hazard_pointers_thread, (void*)0x1, IGNORED_ARG, IGNORED_ARG));

    // act
    result = clds_hash_table_remove(hash_table, hazard_pointers_thread, (void*)0x1, &removed_item, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_REMOVE_RESULT, CLDS_HASH_TABLE_REMOVE_OK, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    CLDS_HASH_TABLE_NODE_RELEASE(TEST_ITEM, removed_item);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_067: [ For each remove the order of the operation shall be computed by passing sequence_number to clds_sorted_list_remove_key. ]*/
TEST_FUNCTION(clds_hash_table_remove_removes_the_key_from_the_list_with_non_NULL_sequence_no)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_HASH_TABLE_REMOVE_RESULT result;
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* removed_item;
    volatile_atomic int64_t sequence_number = 42;
    int64_t remove_seq_no;
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, &sequence_number, NULL, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(clds_hazard_pointers_acquire(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_release(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_reclaim(IGNORED_ARG, IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x1));
    STRICT_EXPECTED_CALL(clds_sorted_list_remove_key(IGNORED_ARG, hazard_pointers_thread, (void*)0x1, IGNORED_ARG, &remove_seq_no));

    // act
    result = clds_hash_table_remove(hash_table, hazard_pointers_thread, (void*)0x1, &removed_item, &remove_seq_no);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_REMOVE_RESULT, CLDS_HASH_TABLE_REMOVE_OK, result);
    ASSERT_ARE_EQUAL(int64_t, 44, remove_seq_no);

    // cleanup
    clds_hash_table_destroy(hash_table);
    CLDS_HASH_TABLE_NODE_RELEASE(TEST_ITEM, removed_item);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_050: [ If clds_hash_table is NULL, clds_hash_table_remove shall fail and return CLDS_HASH_TABLE_REMOVE_ERROR. ]*/
TEST_FUNCTION(clds_hash_table_remove_with_NULL_hash_table_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_REMOVE_RESULT result;
    CLDS_HASH_TABLE_ITEM* removed_item;
    umock_c_reset_all_calls();

    // act
    result = clds_hash_table_remove(NULL, hazard_pointers_thread, (void*)0x1, &removed_item, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_REMOVE_RESULT, CLDS_HASH_TABLE_REMOVE_ERROR, result);

    // cleanup
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_052: [ If clds_hazard_pointers_thread is NULL, clds_hash_table_remove shall fail and return CLDS_HASH_TABLE_REMOVE_ERROR. ]*/
TEST_FUNCTION(clds_hash_table_remove_with_NULL_clds_hazard_pointers_thread_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_HASH_TABLE_REMOVE_RESULT result;
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* removed_item;
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, NULL, NULL, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL);
    umock_c_reset_all_calls();

    // act
    result = clds_hash_table_remove(hash_table, NULL, (void*)0x1, &removed_item, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_REMOVE_RESULT, CLDS_HASH_TABLE_REMOVE_ERROR, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_051: [ If key is NULL, clds_hash_table_remove shall fail and return CLDS_HASH_TABLE_REMOVE_ERROR. ]*/
TEST_FUNCTION(clds_hash_table_remove_with_NULL_key_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_HASH_TABLE_REMOVE_RESULT result;
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* removed_item;
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, NULL, NULL, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL);
    umock_c_reset_all_calls();

    // act
    result = clds_hash_table_remove(hash_table, hazard_pointers_thread, NULL, &removed_item, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_REMOVE_RESULT, CLDS_HASH_TABLE_REMOVE_ERROR, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_056: [ If item is NULL, clds_hash_table_remove shall fail and return CLDS_HASH_TABLE_REMOVE_ERROR. ]*/
TEST_FUNCTION(clds_hash_table_remove_with_NULL_item_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_HASH_TABLE_REMOVE_RESULT result;
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, NULL, NULL, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL);
    umock_c_reset_all_calls();

    // act
    result = clds_hash_table_remove(hash_table, hazard_pointers_thread, (void*)0x1, NULL, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_REMOVE_RESULT, CLDS_HASH_TABLE_REMOVE_ERROR, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_053: [ If the desired key is not found in the hash table (not found in any of the arrays of buckets), clds_hash_table_remove shall return CLDS_HASH_TABLE_REMOVE_NOT_FOUND. ]*/
TEST_FUNCTION(clds_hash_table_remove_with_a_key_that_is_not_in_the_hash_table_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_HASH_TABLE_REMOVE_RESULT result;
    CLDS_HASH_TABLE_ITEM* removed_item;
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, NULL, NULL, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x1));

    // act
    result = clds_hash_table_remove(hash_table, hazard_pointers_thread, (void*)0x1, &removed_item, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_REMOVE_RESULT, CLDS_HASH_TABLE_REMOVE_NOT_FOUND, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_053: [ If the desired key is not found in the hash table (not found in any of the arrays of buckets), clds_hash_table_remove shall return CLDS_HASH_TABLE_REMOVE_NOT_FOUND. ]*/
TEST_FUNCTION(clds_hash_table_remove_with_a_key_that_is_already_deleted_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_HASH_TABLE_REMOVE_RESULT result;
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* removed_item;
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, NULL, NULL, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL);
    (void)clds_hash_table_delete(hash_table, hazard_pointers_thread, (void*)0x1, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x1));

    // act
    result = clds_hash_table_remove(hash_table, hazard_pointers_thread, (void*)0x1, &removed_item, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_REMOVE_RESULT, CLDS_HASH_TABLE_REMOVE_NOT_FOUND, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_054: [ If a bucket is identified and the delete of the item from the underlying list fails, clds_hash_table_remove shall fail and return CLDS_HASH_TABLE_REMOVE_ERROR. ]*/
TEST_FUNCTION(when_the_underlying_delete_from_list_fails_clds_hash_table_remove_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_HASH_TABLE_REMOVE_RESULT result;
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* removed_item;
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, NULL, NULL, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x1));
    STRICT_EXPECTED_CALL(clds_sorted_list_remove_key(IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG))
        .SetReturn(CLDS_SORTED_LIST_REMOVE_ERROR);

    // act
    result = clds_hash_table_remove(hash_table, hazard_pointers_thread, (void*)0x1, &removed_item, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_REMOVE_RESULT, CLDS_HASH_TABLE_REMOVE_ERROR, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_055: [ If the element to be deleted is not found in the biggest array of buckets, then it shall be looked up in the next available array of buckets. ]*/
TEST_FUNCTION(remove_looks_in_2_buckets)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_HASH_TABLE_REMOVE_RESULT result;
    CLDS_HASH_TABLE_ITEM* item_1 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* item_2 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* removed_item;
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 1, hazard_pointers, NULL, NULL, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item_1, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x2, item_2, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(clds_hazard_pointers_acquire(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_release(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_reclaim(IGNORED_ARG, IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x1));
    // due to resize it is only one
    STRICT_EXPECTED_CALL(clds_sorted_list_remove_key(IGNORED_ARG, IGNORED_ARG, (void*)0x1, IGNORED_ARG, IGNORED_ARG));

    // act
    result = clds_hash_table_remove(hash_table, hazard_pointers_thread, (void*)0x1, &removed_item, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_REMOVE_RESULT, CLDS_HASH_TABLE_REMOVE_OK, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    CLDS_HASH_TABLE_NODE_RELEASE(TEST_ITEM, removed_item);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_055: [ If the element to be deleted is not found in the biggest array of buckets, then it shall be looked up in the next available array of buckets. ]*/
TEST_FUNCTION(remove_looks_in_3_buckets)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_HASH_TABLE_REMOVE_RESULT result;
    // 1st bucket array has 1 bucket
    CLDS_HASH_TABLE_ITEM* item_1 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    // 2nd bucket array has 2 buckets
    CLDS_HASH_TABLE_ITEM* item_2 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* item_3 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    // 3rd bucket array has 4 buckets
    CLDS_HASH_TABLE_ITEM* item_4 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* removed_item;
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 1, hazard_pointers, NULL, NULL, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item_1, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x2, item_2, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x3, item_3, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x4, item_4, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(clds_hazard_pointers_acquire(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_release(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_reclaim(IGNORED_ARG, IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x1));
    // due to resize, only too lists are there
    STRICT_EXPECTED_CALL(clds_sorted_list_remove_key(IGNORED_ARG, IGNORED_ARG, (void*)0x1, IGNORED_ARG, IGNORED_ARG));
    STRICT_EXPECTED_CALL(clds_sorted_list_remove_key(IGNORED_ARG, IGNORED_ARG, (void*)0x1, IGNORED_ARG, IGNORED_ARG));

    // act
    result = clds_hash_table_remove(hash_table, hazard_pointers_thread, (void*)0x1, &removed_item, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_REMOVE_RESULT, CLDS_HASH_TABLE_REMOVE_OK, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    CLDS_HASH_TABLE_NODE_RELEASE(TEST_ITEM, removed_item);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_053: [ If the desired key is not found in the hash table (not found in any of the arrays of buckets), clds_hash_table_remove shall return CLDS_HASH_TABLE_REMOVE_NOT_FOUND. ]*/
TEST_FUNCTION(when_item_is_not_found_in_any_bucket_remove_returns_NOT_FOUND)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_HASH_TABLE_REMOVE_RESULT result;
    CLDS_HASH_TABLE_ITEM* item_1 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* item_2 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* removed_item;
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 1, hazard_pointers, NULL, NULL, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item_1, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x2, item_2, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(clds_hazard_pointers_acquire(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_release(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_reclaim(IGNORED_ARG, IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x3));
    STRICT_EXPECTED_CALL(clds_sorted_list_remove_key(IGNORED_ARG, IGNORED_ARG, (void*)0x3, IGNORED_ARG, IGNORED_ARG));

    // act
    result = clds_hash_table_remove(hash_table, hazard_pointers_thread, (void*)0x3, &removed_item, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_REMOVE_RESULT, CLDS_HASH_TABLE_REMOVE_NOT_FOUND, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_070: [ If the sequence_number argument is non-NULL, but no start sequence number was specified in clds_hash_table_create, clds_hash_table_remove shall fail and return CLDS_HASH_TABLE_REMOVE_ERROR. ]*/
TEST_FUNCTION(clds_hash_table_remove_with_non_NULL_sequence_no_and_NULL_start_sequence_no_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_HASH_TABLE_REMOVE_RESULT result;
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* removed_item;
    int64_t remove_seq_no;
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, NULL, NULL, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL);
    umock_c_reset_all_calls();

    // act
    result = clds_hash_table_remove(hash_table, hazard_pointers_thread, (void*)0x1, &removed_item, &remove_seq_no);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_REMOVE_RESULT, CLDS_HASH_TABLE_REMOVE_ERROR, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* clds_hash_table_find */

/* Tests_SRS_CLDS_HASH_TABLE_01_034: [ clds_hash_table_find shall find the key identified by key in the hash table and on success return the item corresponding to it. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_040: [ clds_hash_table_find shall hash the key by calling the compute_hash function passed to clds_hash_table_create. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_041: [ clds_hash_table_find shall look up the key in the biggest array of buckets. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_044: [ Looking up the key in the array of buckets is done by obtaining the list in the bucket correspoding to the hash and looking up the key in the list by calling clds_sorted_list_find. ]*/
TEST_FUNCTION(clds_hash_table_find_succeeds)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_HASH_TABLE_ITEM* result;
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 3, hazard_pointers, NULL, NULL, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(clds_hazard_pointers_acquire(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_release(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_reclaim(IGNORED_ARG, IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x1));
    STRICT_EXPECTED_CALL(clds_sorted_list_find_key(IGNORED_ARG, IGNORED_ARG, (void*)0x1));

    // act
    result = clds_hash_table_find(hash_table, hazard_pointers_thread, (void*)0x1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(void_ptr, (void*)item, (void*)result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    CLDS_HASH_TABLE_NODE_RELEASE(TEST_ITEM, result);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_034: [ clds_hash_table_find shall find the key identified by key in the hash table and on success return the item corresponding to it. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_040: [ clds_hash_table_find shall hash the key by calling the compute_hash function passed to clds_hash_table_create. ]*/
TEST_FUNCTION(clds_hash_table_find_2nd_item_out_of_3_succeeds)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_HASH_TABLE_ITEM* result;
    CLDS_HASH_TABLE_ITEM* item_1 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* item_2 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* item_3 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 3, hazard_pointers, NULL, NULL, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item_1, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x2, item_2, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x3, item_3, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(clds_hazard_pointers_acquire(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_release(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_reclaim(IGNORED_ARG, IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x2));
    STRICT_EXPECTED_CALL(clds_sorted_list_find_key(IGNORED_ARG, IGNORED_ARG, (void*)0x2));

    // act
    result = clds_hash_table_find(hash_table, hazard_pointers_thread, (void*)0x2);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(void_ptr, (void*)item_2, (void*)result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    CLDS_HASH_TABLE_NODE_RELEASE(TEST_ITEM, result);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_034: [ clds_hash_table_find shall find the key identified by key in the hash table and on success return the item corresponding to it. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_040: [ clds_hash_table_find shall hash the key by calling the compute_hash function passed to clds_hash_table_create. ]*/
TEST_FUNCTION(clds_hash_table_find_3rd_item_out_of_3_succeeds)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_HASH_TABLE_ITEM* result;
    CLDS_HASH_TABLE_ITEM* item_1 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* item_2 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* item_3 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 3, hazard_pointers, NULL, NULL, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item_1, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x2, item_2, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x3, item_3, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(clds_hazard_pointers_acquire(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_release(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_reclaim(IGNORED_ARG, IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x3));
    STRICT_EXPECTED_CALL(clds_sorted_list_find_key(IGNORED_ARG, IGNORED_ARG, (void*)0x3));

    // act
    result = clds_hash_table_find(hash_table, hazard_pointers_thread, (void*)0x3);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(void_ptr, (void*)item_3, (void*)result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    CLDS_HASH_TABLE_NODE_RELEASE(TEST_ITEM, result);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_035: [ If clds_hash_table is NULL, clds_hash_table_find shall fail and return NULL. ]*/
TEST_FUNCTION(clds_hash_table_find_with_NULL_hash_table_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_ITEM* result;
    umock_c_reset_all_calls();

    // act
    result = clds_hash_table_find(NULL, hazard_pointers_thread, (void*)0x3);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);

    // cleanup
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_036: [ If clds_hazard_pointers_thread is NULL, clds_hash_table_find shall fail and return NULL. ]*/
TEST_FUNCTION(clds_hash_table_find_with_NULL_clds_hazard_pointers_thread_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_HASH_TABLE_ITEM* result;
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 1, hazard_pointers, NULL, NULL, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL);
    umock_c_reset_all_calls();

    // act
    result = clds_hash_table_find(hash_table, NULL, (void*)0x3);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_037: [ If key is NULL, clds_hash_table_find shall fail and return NULL. ]*/
TEST_FUNCTION(clds_hash_table_find_with_NULL_key_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_HASH_TABLE_ITEM* result;
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 1, hazard_pointers, NULL, NULL, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL);
    umock_c_reset_all_calls();

    // act
    result = clds_hash_table_find(hash_table, hazard_pointers_thread, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_042: [ If the key is not found in the biggest array of buckets, the next bucket arrays shall be looked up. ]*/
TEST_FUNCTION(clds_hash_table_find_looks_up_in_the_2nd_array_of_buckets)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_HASH_TABLE_ITEM* result;
    CLDS_HASH_TABLE_ITEM* item_1 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* item_2 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* item_3 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 1, hazard_pointers, NULL, NULL, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item_1, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x2, item_2, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x3, item_3, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(clds_hazard_pointers_acquire(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_release(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_reclaim(IGNORED_ARG, IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x1));
    STRICT_EXPECTED_CALL(clds_sorted_list_find_key(IGNORED_ARG, IGNORED_ARG, (void*)0x1));
    STRICT_EXPECTED_CALL(clds_sorted_list_find_key(IGNORED_ARG, IGNORED_ARG, (void*)0x1));

    // act
    result = clds_hash_table_find(hash_table, hazard_pointers_thread, (void*)0x1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(void_ptr, (void*)item_1, (void*)result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    CLDS_HASH_TABLE_NODE_RELEASE(TEST_ITEM, result);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_043: [ If the key is not found at all, clds_hash_table_find shall return NULL. ]*/
TEST_FUNCTION(when_key_is_not_found_in_any_bucket_levels_clds_hash_table_find_yields_NULL)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_HASH_TABLE_ITEM* result;
    CLDS_HASH_TABLE_ITEM* item_1 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* item_2 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* item_3 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 1, hazard_pointers, NULL, NULL, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item_1, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x2, item_2, NULL);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x3, item_3, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(clds_hazard_pointers_acquire(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_release(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_reclaim(IGNORED_ARG, IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x4));
    STRICT_EXPECTED_CALL(clds_sorted_list_find_key(IGNORED_ARG, IGNORED_ARG, (void*)0x4));
    STRICT_EXPECTED_CALL(clds_sorted_list_find_key(IGNORED_ARG, IGNORED_ARG, (void*)0x4));

    // act
    result = clds_hash_table_find(hash_table, hazard_pointers_thread, (void*)0x4);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* clds_hash_table_set_value */

/* Tests_SRS_CLDS_HASH_TABLE_01_079: [ If clds_hash_table is NULL, clds_hash_table_set_value shall fail and return CLDS_HASH_TABLE_SET_VALUE_ERROR. ]*/
TEST_FUNCTION(clds_hash_table_set_value_with_NULL_hash_table_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_ITEM* item_1 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* old_item;
    CLDS_HASH_TABLE_SET_VALUE_RESULT result;
    int64_t set_value_seq_no;
    umock_c_reset_all_calls();

    // act
    result = clds_hash_table_set_value(NULL, hazard_pointers_thread, (void*)0x4, item_1, NULL, NULL, &old_item, &set_value_seq_no);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_SET_VALUE_RESULT, CLDS_HASH_TABLE_SET_VALUE_ERROR, result);

    // cleanup
    clds_hazard_pointers_destroy(hazard_pointers);
    CLDS_HASH_TABLE_NODE_RELEASE(TEST_ITEM, item_1);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_080: [ If clds_hazard_pointers_thread is NULL, clds_hash_table_set_value shall fail and return CLDS_HASH_TABLE_SET_VALUE_ERROR. ]*/
TEST_FUNCTION(clds_hash_table_set_value_with_NULL_clds_hazard_pointers_thread_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HASH_TABLE_ITEM* item_1 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* old_item;
    CLDS_HASH_TABLE_SET_VALUE_RESULT result;
    int64_t set_value_seq_no;
    volatile_atomic int64_t sequence_number;
    (void)interlocked_exchange_64(&sequence_number, 42);
    CLDS_HASH_TABLE_HANDLE hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 1, hazard_pointers, &sequence_number, NULL, NULL);
    umock_c_reset_all_calls();

    // act
    result = clds_hash_table_set_value(hash_table, NULL, (void*)0x4, item_1, NULL, NULL, &old_item, &set_value_seq_no);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_SET_VALUE_RESULT, CLDS_HASH_TABLE_SET_VALUE_ERROR, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
    CLDS_HASH_TABLE_NODE_RELEASE(TEST_ITEM, item_1);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_081: [ If key is NULL, clds_hash_table_set_value shall fail and return CLDS_HASH_TABLE_SET_VALUE_ERROR. ]*/
TEST_FUNCTION(clds_hash_table_set_value_with_NULL_key_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_ITEM* item_1 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* old_item;
    CLDS_HASH_TABLE_SET_VALUE_RESULT result;
    int64_t set_value_seq_no;
    volatile_atomic int64_t sequence_number;
    (void)interlocked_exchange_64(&sequence_number, 42);
    CLDS_HASH_TABLE_HANDLE hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 1, hazard_pointers, &sequence_number, NULL, NULL);
    umock_c_reset_all_calls();

    // act
    result = clds_hash_table_set_value(hash_table, hazard_pointers_thread, NULL, item_1, NULL, NULL, &old_item, &set_value_seq_no);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_SET_VALUE_RESULT, CLDS_HASH_TABLE_SET_VALUE_ERROR, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
    CLDS_HASH_TABLE_NODE_RELEASE(TEST_ITEM, item_1);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_082: [ If new_item is NULL, clds_hash_table_set_value shall fail and return CLDS_HASH_TABLE_SET_VALUE_ERROR. ]*/
TEST_FUNCTION(clds_hash_table_set_value_with_NULL_new_item_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_ITEM* old_item;
    CLDS_HASH_TABLE_SET_VALUE_RESULT result;
    int64_t set_value_seq_no;
    volatile_atomic int64_t sequence_number;
    (void)interlocked_exchange_64(&sequence_number, 42);
    CLDS_HASH_TABLE_HANDLE hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 1, hazard_pointers, &sequence_number, NULL, NULL);
    umock_c_reset_all_calls();

    // act
    result = clds_hash_table_set_value(hash_table, hazard_pointers_thread, (void*)0x4, NULL, NULL, NULL, &old_item, &set_value_seq_no);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_SET_VALUE_RESULT, CLDS_HASH_TABLE_SET_VALUE_ERROR, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_083: [ If old_item is NULL, clds_hash_table_set_value shall fail and return CLDS_HASH_TABLE_SET_VALUE_ERROR. ]*/
TEST_FUNCTION(clds_hash_table_set_value_with_NULL_old_item_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_ITEM* item_1 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_SET_VALUE_RESULT result;
    int64_t set_value_seq_no;
    volatile_atomic int64_t sequence_number;
    (void)interlocked_exchange_64(&sequence_number, 42);
    CLDS_HASH_TABLE_HANDLE hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 1, hazard_pointers, &sequence_number, NULL, NULL);
    umock_c_reset_all_calls();

    // act
    result = clds_hash_table_set_value(hash_table, hazard_pointers_thread, (void*)0x4, item_1, NULL, NULL, NULL, &set_value_seq_no);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_SET_VALUE_RESULT, CLDS_HASH_TABLE_SET_VALUE_ERROR, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
    CLDS_HASH_TABLE_NODE_RELEASE(TEST_ITEM, item_1);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_084: [ If the sequence_number argument is non-NULL, but no start sequence number was specified in clds_hash_table_create, clds_hash_table_set_value shall fail and return CLDS_HASH_TABLE_SET_VALUE_ERROR. ]*/
TEST_FUNCTION(clds_hash_table_set_value_with_non_NULL_sequence_number_when_a_starting_sequence_number_was_not_specified)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_ITEM* item_1 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* old_item;
    CLDS_HASH_TABLE_SET_VALUE_RESULT result;
    int64_t set_value_seq_no;
    CLDS_HASH_TABLE_HANDLE hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 1, hazard_pointers, NULL, NULL, NULL);
    umock_c_reset_all_calls();

    // act
    result = clds_hash_table_set_value(hash_table, hazard_pointers_thread, (void*)0x4, item_1, NULL, NULL, &old_item, &set_value_seq_no);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_SET_VALUE_RESULT, CLDS_HASH_TABLE_SET_VALUE_ERROR, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
    CLDS_HASH_TABLE_NODE_RELEASE(TEST_ITEM, item_1);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_085: [ clds_hash_table_set_value shall go through all non top level bucket arrays and: ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_102: [ If the key is not found in any of the non top level buckets arrays, clds_hash_table_set_value: ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_103: [ clds_hash_table_set_value shall obtain the sorted list at the bucket corresponding to the hash of the key. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_104: [  If no list exists at the designated bucket, one shall be created. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_105: [ clds_hash_table_set_value shall call clds_hash_table_set_value on the top level bucket array, passing key, new_item, condition_check_func, condition_check_context, old_item and only_if_exists set to false. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_099: [ If clds_sorted_list_set_value returns CLDS_SORTED_LIST_SET_VALUE_OK, clds_hash_table_set_value shall succeed and return CLDS_HASH_TABLE_SET_VALUE_OK. ]*/
TEST_FUNCTION(clds_hash_table_set_value_with_empty_hash_table_sets_the_value_on_the_top_level_buckets_array)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_SORTED_LIST_HANDLE linked_list;
    CLDS_HASH_TABLE_SET_VALUE_RESULT result;
    CLDS_HASH_TABLE_ITEM* old_item;
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, NULL, NULL, NULL);
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x1));
    STRICT_EXPECTED_CALL(clds_sorted_list_create(hazard_pointers, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, NULL, IGNORED_ARG, IGNORED_ARG))
        .CaptureReturn(&linked_list);
    STRICT_EXPECTED_CALL(clds_sorted_list_set_value(IGNORED_ARG, IGNORED_ARG, (void*)0x1, (CLDS_SORTED_LIST_ITEM*)item, NULL, NULL, IGNORED_ARG, NULL, false))
        .ValidateArgumentValue_clds_sorted_list(&linked_list);

    // act
    result = clds_hash_table_set_value(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL, NULL, &old_item, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_SET_VALUE_RESULT, CLDS_HASH_TABLE_SET_VALUE_OK, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_085: [ clds_hash_table_set_value shall go through all non top level bucket arrays and: ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_102: [ If the key is not found in any of the non top level buckets arrays, clds_hash_table_set_value: ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_103: [ clds_hash_table_set_value shall obtain the sorted list at the bucket corresponding to the hash of the key. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_104: [  If no list exists at the designated bucket, one shall be created. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_105: [ clds_hash_table_set_value shall call clds_hash_table_set_value on the top level bucket array, passing key, new_item, condition_check_func, condition_check_context, old_item and only_if_exists set to false. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_099: [ If clds_sorted_list_set_value returns CLDS_SORTED_LIST_SET_VALUE_OK, clds_hash_table_set_value shall succeed and return CLDS_HASH_TABLE_SET_VALUE_OK. ]*/
TEST_FUNCTION(clds_hash_table_set_value_with_empty_hash_table_sets_the_value_on_the_top_level_buckets_array_with_condition_check)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_SORTED_LIST_HANDLE linked_list;
    CLDS_HASH_TABLE_SET_VALUE_RESULT result;
    CLDS_HASH_TABLE_ITEM* old_item;
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, NULL, NULL, NULL);
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x1));
    STRICT_EXPECTED_CALL(clds_sorted_list_create(hazard_pointers, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, NULL, IGNORED_ARG, IGNORED_ARG))
        .CaptureReturn(&linked_list);
    STRICT_EXPECTED_CALL(clds_sorted_list_set_value(IGNORED_ARG, IGNORED_ARG, (void*)0x1, (CLDS_SORTED_LIST_ITEM*)item, test_item_condition_check, (void*)0x42, IGNORED_ARG, NULL, false))
        .ValidateArgumentValue_clds_sorted_list(&linked_list);

    // act
    result = clds_hash_table_set_value(hash_table, hazard_pointers_thread, (void*)0x1, item, test_item_condition_check, (void*)0x42, &old_item, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_SET_VALUE_RESULT, CLDS_HASH_TABLE_SET_VALUE_OK, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

static void test_when_clds_sorted_list_set_value_fails_clds_hash_table_set_value_with_empty_hash_table_fails(
    CLDS_SORTED_LIST_SET_VALUE_RESULT sorted_list_result, CLDS_HASH_TABLE_SET_VALUE_RESULT set_value_result,
    CONDITION_CHECK_CB condition_check_cb, void* condition_check_context)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_SORTED_LIST_HANDLE linked_list;
    CLDS_HASH_TABLE_SET_VALUE_RESULT result;
    CLDS_HASH_TABLE_ITEM* old_item;
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, NULL, NULL, NULL);
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x1));
    STRICT_EXPECTED_CALL(clds_sorted_list_create(hazard_pointers, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, NULL, IGNORED_ARG, IGNORED_ARG))
        .CaptureReturn(&linked_list);
    STRICT_EXPECTED_CALL(clds_sorted_list_set_value(IGNORED_ARG, IGNORED_ARG, (void*)0x1, (CLDS_SORTED_LIST_ITEM*)item, condition_check_cb, condition_check_context, IGNORED_ARG, NULL, false))
        .ValidateArgumentValue_clds_sorted_list(&linked_list)
        .SetReturn(sorted_list_result);

    // act
    result = clds_hash_table_set_value(hash_table, hazard_pointers_thread, (void*)0x1, item, condition_check_cb, condition_check_context, &old_item, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_SET_VALUE_RESULT, set_value_result, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
    CLDS_HASH_TABLE_NODE_RELEASE(TEST_ITEM, item);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_100: [ If clds_sorted_list_set_value returns any other value, clds_hash_table_set_value shall fail and return CLDS_HASH_TABLE_SET_VALUE_ERROR. ]*/
TEST_FUNCTION(when_clds_sorted_list_set_value_fails_with_ERROR_clds_hash_table_set_value_with_empty_hash_table_fails)
{
    test_when_clds_sorted_list_set_value_fails_clds_hash_table_set_value_with_empty_hash_table_fails(CLDS_SORTED_LIST_SET_VALUE_ERROR, CLDS_HASH_TABLE_SET_VALUE_ERROR, NULL, NULL);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_100: [ If clds_sorted_list_set_value returns any other value, clds_hash_table_set_value shall fail and return CLDS_HASH_TABLE_SET_VALUE_ERROR. ]*/
TEST_FUNCTION(when_clds_sorted_list_set_value_fails_with_NOT_FOUND_clds_hash_table_set_value_with_empty_hash_table_fails)
{
    test_when_clds_sorted_list_set_value_fails_clds_hash_table_set_value_with_empty_hash_table_fails(CLDS_SORTED_LIST_SET_VALUE_NOT_FOUND, CLDS_HASH_TABLE_SET_VALUE_ERROR, NULL, NULL);
}

/* Tests_SRS_CLDS_HASH_TABLE_04_002: [ If clds_sorted_list_set_value returns CLDS_SORTED_LIST_SET_VALUE_CONDITION_NOT_MET, clds_hash_table_set_value shall fail and return CLDS_HASH_TABLE_SET_VALUE_CONDITION_NOT_MET. ]*/
TEST_FUNCTION(when_clds_sorted_list_set_value_fails_with_CONDITION_NOT_MET_clds_hash_table_set_value_with_empty_hash_table_fails)
{
    test_when_clds_sorted_list_set_value_fails_clds_hash_table_set_value_with_empty_hash_table_fails(CLDS_SORTED_LIST_SET_VALUE_CONDITION_NOT_MET, CLDS_HASH_TABLE_SET_VALUE_CONDITION_NOT_MET, test_item_condition_check, (void*)0x42);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_106: [ If any error occurs, clds_hash_table_set_value shall fail and return CLDS_HASH_TABLE_SET_VALUE_ERROR. ]*/
TEST_FUNCTION(when_underlying_calls_fail_clds_hash_table_set_value_with_empty_hash_table_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_SORTED_LIST_HANDLE linked_list;
    CLDS_HASH_TABLE_SET_VALUE_RESULT result;
    CLDS_HASH_TABLE_ITEM* old_item;
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, NULL, NULL, NULL);
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x1))
        .CallCannotFail();
    STRICT_EXPECTED_CALL(clds_sorted_list_create(hazard_pointers, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, NULL, IGNORED_ARG, IGNORED_ARG))
        .CaptureReturn(&linked_list)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(clds_sorted_list_set_value(IGNORED_ARG, IGNORED_ARG, (void*)0x1, (CLDS_SORTED_LIST_ITEM*)item, NULL, NULL, IGNORED_ARG, NULL, false))
        .ValidateArgumentValue_clds_sorted_list(&linked_list)
        .SetFailReturn(CLDS_SORTED_LIST_SET_VALUE_ERROR);

    umock_c_negative_tests_snapshot();

    for (size_t i = 0; i < umock_c_negative_tests_call_count(); i++)
    {
        if (umock_c_negative_tests_can_call_fail(i))
        {
            umock_c_negative_tests_reset();
            umock_c_negative_tests_fail_call(i);

            // act
            result = clds_hash_table_set_value(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL, NULL, &old_item, NULL);

            // assert
            ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_SET_VALUE_RESULT, CLDS_HASH_TABLE_SET_VALUE_ERROR, result, "On failed call %zu", i);
        }
    }

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
    CLDS_HASH_TABLE_NODE_RELEASE(TEST_ITEM, item);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_107: [ If there is no sorted list in the bucket identified by the hash of the key, clds_hash_table_set_value shall advance to the next level of buckets. ]*/
TEST_FUNCTION(when_there_is_no_sorted_list_in_lower_level_bucket_no_find_is_done_by_clds_hash_table_set_value)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_HASH_TABLE_SET_VALUE_RESULT result;
    CLDS_HASH_TABLE_ITEM* old_item;
    CLDS_SORTED_LIST_HANDLE linked_list;
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 4, hazard_pointers, NULL, NULL, NULL);
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_INSERT_RESULT, CLDS_HASH_TABLE_INSERT_OK, clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL));
    size_t i;

    CLDS_HASH_TABLE_ITEM* dummy_items[4];
    for (i = 0; i < 4; i++)
    {
        dummy_items[i] = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
        ASSERT_IS_NOT_NULL(dummy_items[i]);
        ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_INSERT_RESULT, CLDS_HASH_TABLE_INSERT_OK, clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)((4 * i) + 2), dummy_items[i], NULL));
    }

    CLDS_HASH_TABLE_ITEM* new_item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    ASSERT_IS_NOT_NULL(new_item);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(clds_hazard_pointers_acquire(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_release(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x3));
    STRICT_EXPECTED_CALL(clds_sorted_list_create(hazard_pointers, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, NULL, IGNORED_ARG, IGNORED_ARG))
        .CaptureReturn(&linked_list);
    STRICT_EXPECTED_CALL(clds_sorted_list_set_value(IGNORED_ARG, IGNORED_ARG, (void*)0x3, (CLDS_SORTED_LIST_ITEM*)new_item, NULL, NULL, IGNORED_ARG, NULL, false))
        .ValidateArgumentValue_clds_sorted_list(&linked_list);

    // act
    result = clds_hash_table_set_value(hash_table, hazard_pointers_thread, (void*)0x3, new_item, NULL, NULL, &old_item, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_SET_VALUE_RESULT, CLDS_HASH_TABLE_SET_VALUE_OK, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_108: [ If there is a sorted list in the bucket identified by the hash of the key, clds_hash_table_set_value shall find the key in the list. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_109: [ If the key is not found, clds_hash_table_set_value shall advance to the next level of buckets. ]*/
TEST_FUNCTION(clds_hash_table_set_value_with_an_existing_item_in_the_lower_level_bucket_looks_for_the_item_in_the_lower_levels)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_SORTED_LIST_HANDLE linked_list;
    CLDS_HASH_TABLE_SET_VALUE_RESULT result;
    CLDS_HASH_TABLE_ITEM* old_item;
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 1, hazard_pointers, NULL, NULL, NULL);
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* item_2 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* item_3 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_INSERT_RESULT, CLDS_HASH_TABLE_INSERT_OK, clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL));
    // add one filler item to expand the hash table
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_INSERT_RESULT, CLDS_HASH_TABLE_INSERT_OK, clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x3, item_2, NULL));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(clds_hazard_pointers_acquire(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_release(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x2));
    STRICT_EXPECTED_CALL(clds_sorted_list_find_key(IGNORED_ARG, IGNORED_ARG, (void*)0x2));
    STRICT_EXPECTED_CALL(clds_sorted_list_create(hazard_pointers, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, NULL, IGNORED_ARG, IGNORED_ARG))
        .CaptureReturn(&linked_list);
    STRICT_EXPECTED_CALL(clds_sorted_list_set_value(IGNORED_ARG, IGNORED_ARG, (void*)0x2, (CLDS_SORTED_LIST_ITEM*)item_3, NULL, NULL, IGNORED_ARG, NULL, false))
        .ValidateArgumentValue_clds_sorted_list(&linked_list);

    // act
    result = clds_hash_table_set_value(hash_table, hazard_pointers_thread, (void*)0x2, item_3, NULL, NULL, &old_item, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_SET_VALUE_RESULT, CLDS_HASH_TABLE_SET_VALUE_OK, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_110: [ If the key is found, clds_hash_table_set_value shall call clds_sorted_list_set_value with the key, new_item, condition_check_func, condition_check_context and old_item and only_if_exists set to true. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_112: [ If clds_sorted_list_set_value succeeds, clds_hash_table_set_value shall return CLDS_HASH_TABLE_SET_VALUE_OK. ]*/
TEST_FUNCTION(clds_hash_table_set_value_with_an_existing_item_in_the_lower_level_finds_the_item_and_replaces_it_in_the_lower_levels)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_HASH_TABLE_SET_VALUE_RESULT result;
    CLDS_HASH_TABLE_ITEM* old_item;
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 1, hazard_pointers, NULL, NULL, NULL);
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* item_2 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* item_3 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_INSERT_RESULT, CLDS_HASH_TABLE_INSERT_OK, clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL));
    // add one filler item to expand the hash table
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_INSERT_RESULT, CLDS_HASH_TABLE_INSERT_OK, clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x3, item_2, NULL));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(clds_hazard_pointers_acquire(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_release(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_reclaim(IGNORED_ARG, IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x1));
    STRICT_EXPECTED_CALL(clds_sorted_list_find_key(IGNORED_ARG, IGNORED_ARG, (void*)0x1));
    STRICT_EXPECTED_CALL(clds_sorted_list_node_release(IGNORED_ARG));
    STRICT_EXPECTED_CALL(clds_sorted_list_set_value(IGNORED_ARG, IGNORED_ARG, (void*)0x1, (CLDS_SORTED_LIST_ITEM*)item_3, NULL, NULL, IGNORED_ARG, NULL, true));

    // act
    result = clds_hash_table_set_value(hash_table, hazard_pointers_thread, (void*)0x1, item_3, NULL, NULL, &old_item, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_SET_VALUE_RESULT, CLDS_HASH_TABLE_SET_VALUE_OK, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
    CLDS_HASH_TABLE_NODE_RELEASE(TEST_ITEM, old_item);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_110: [ If the key is found, clds_hash_table_set_value shall call clds_sorted_list_set_value with the key, new_item, condition_check_func, condition_check_context and old_item and only_if_exists set to true. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_112: [ If clds_sorted_list_set_value succeeds, clds_hash_table_set_value shall return CLDS_HASH_TABLE_SET_VALUE_OK. ]*/
TEST_FUNCTION(clds_hash_table_set_value_with_condition_check_cb_has_cb_called)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_HASH_TABLE_SET_VALUE_RESULT result;
    CLDS_HASH_TABLE_ITEM* old_item;
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 1, hazard_pointers, NULL, NULL, NULL);
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* item_2 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* item_3 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_INSERT_RESULT, CLDS_HASH_TABLE_INSERT_OK, clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL));
    // add one filler item to expand the hash table
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_INSERT_RESULT, CLDS_HASH_TABLE_INSERT_OK, clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x3, item_2, NULL));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(clds_hazard_pointers_acquire(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_release(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_reclaim(IGNORED_ARG, IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x1));
    STRICT_EXPECTED_CALL(clds_sorted_list_find_key(IGNORED_ARG, IGNORED_ARG, (void*)0x1));
    STRICT_EXPECTED_CALL(clds_sorted_list_node_release(IGNORED_ARG));
    STRICT_EXPECTED_CALL(clds_sorted_list_set_value(IGNORED_ARG, IGNORED_ARG, (void*)0x1, (CLDS_SORTED_LIST_ITEM*)item_3, test_item_condition_check, (void*)0x42, IGNORED_ARG, NULL, true));
    STRICT_EXPECTED_CALL(test_item_condition_check((void*)0x42, IGNORED_ARG, IGNORED_ARG));

    // act
    result = clds_hash_table_set_value(hash_table, hazard_pointers_thread, (void*)0x1, item_3, test_item_condition_check, (void*)0x42, &old_item, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_SET_VALUE_RESULT, CLDS_HASH_TABLE_SET_VALUE_OK, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
    CLDS_HASH_TABLE_NODE_RELEASE(TEST_ITEM, old_item);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_110: [ If the key is found, clds_hash_table_set_value shall call clds_sorted_list_set_value with the key, new_item, condition_check_func, condition_check_context and old_item and only_if_exists set to true. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_04_001: [ If clds_sorted_list_set_value returns CLDS_SORTED_LIST_SET_VALUE_CONDITION_NOT_MET, clds_hash_table_set_value shall fail and return CLDS_HASH_TABLE_SET_VALUE_CONDITION_NOT_MET. ]*/
TEST_FUNCTION(clds_hash_table_set_value_fails_when_condition_check_returns_not_met)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_HASH_TABLE_SET_VALUE_RESULT result;
    CLDS_HASH_TABLE_ITEM* old_item;
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 1, hazard_pointers, NULL, NULL, NULL);
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* item_2 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* item_3 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_INSERT_RESULT, CLDS_HASH_TABLE_INSERT_OK, clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL));
    // add one filler item to expand the hash table
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_INSERT_RESULT, CLDS_HASH_TABLE_INSERT_OK, clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x3, item_2, NULL));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(clds_hazard_pointers_acquire(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_release(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_reclaim(IGNORED_ARG, IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x1));
    STRICT_EXPECTED_CALL(clds_sorted_list_find_key(IGNORED_ARG, IGNORED_ARG, (void*)0x1));
    STRICT_EXPECTED_CALL(clds_sorted_list_node_release(IGNORED_ARG));
    STRICT_EXPECTED_CALL(clds_sorted_list_set_value(IGNORED_ARG, IGNORED_ARG, (void*)0x1, (CLDS_SORTED_LIST_ITEM*)item_3, test_item_condition_check, (void*)0x42, IGNORED_ARG, NULL, true));
    g_condition_check_result = CLDS_CONDITION_CHECK_NOT_MET;
    STRICT_EXPECTED_CALL(test_item_condition_check((void*)0x42, IGNORED_ARG, IGNORED_ARG));

    // act
    result = clds_hash_table_set_value(hash_table, hazard_pointers_thread, (void*)0x1, item_3, test_item_condition_check, (void*)0x42, &old_item, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_SET_VALUE_RESULT, CLDS_HASH_TABLE_SET_VALUE_CONDITION_NOT_MET, result);

    // cleanup
    CLDS_HASH_TABLE_NODE_RELEASE(TEST_ITEM, item_3);
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_110: [ If the key is found, clds_hash_table_set_value shall call clds_sorted_list_set_value with the key, new_item, condition_check_func, condition_check_context and old_item and only_if_exists set to true. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_01_111: [ If clds_sorted_list_set_value fails, clds_hash_table_set_value shall fail and return CLDS_HASH_TABLE_SET_VALUE_ERROR. ]*/
TEST_FUNCTION(clds_hash_table_set_value_fails_when_condition_check_returns_error)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_HASH_TABLE_SET_VALUE_RESULT result;
    CLDS_HASH_TABLE_ITEM* old_item;
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 1, hazard_pointers, NULL, NULL, NULL);
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* item_2 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* item_3 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_INSERT_RESULT, CLDS_HASH_TABLE_INSERT_OK, clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL));
    // add one filler item to expand the hash table
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_INSERT_RESULT, CLDS_HASH_TABLE_INSERT_OK, clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x3, item_2, NULL));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(clds_hazard_pointers_acquire(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_release(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_reclaim(IGNORED_ARG, IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x1));
    STRICT_EXPECTED_CALL(clds_sorted_list_find_key(IGNORED_ARG, IGNORED_ARG, (void*)0x1));
    STRICT_EXPECTED_CALL(clds_sorted_list_node_release(IGNORED_ARG));
    STRICT_EXPECTED_CALL(clds_sorted_list_set_value(IGNORED_ARG, IGNORED_ARG, (void*)0x1, (CLDS_SORTED_LIST_ITEM*)item_3, test_item_condition_check, (void*)0x42, IGNORED_ARG, NULL, true));
    g_condition_check_result = CLDS_CONDITION_CHECK_ERROR;
    STRICT_EXPECTED_CALL(test_item_condition_check((void*)0x42, IGNORED_ARG, IGNORED_ARG));

    // act
    result = clds_hash_table_set_value(hash_table, hazard_pointers_thread, (void*)0x1, item_3, test_item_condition_check, (void*)0x42, &old_item, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_SET_VALUE_RESULT, CLDS_HASH_TABLE_SET_VALUE_ERROR, result);

    // cleanup
    CLDS_HASH_TABLE_NODE_RELEASE(TEST_ITEM, item_3);
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_111: [ If clds_sorted_list_set_value fails, clds_hash_table_set_value shall fail and return CLDS_HASH_TABLE_SET_VALUE_ERROR. ]*/
TEST_FUNCTION(when_clds_sorted_list_set_value_in_lower_layer_fails_clds_hash_table_set_value_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    CLDS_HASH_TABLE_SET_VALUE_RESULT result;
    CLDS_HASH_TABLE_ITEM* old_item;
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 1, hazard_pointers, NULL, NULL, NULL);
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* item_2 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    CLDS_HASH_TABLE_ITEM* item_3 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_INSERT_RESULT, CLDS_HASH_TABLE_INSERT_OK, clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL));
    // add one filler item to expand the hash table
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_INSERT_RESULT, CLDS_HASH_TABLE_INSERT_OK, clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x3, item_2, NULL));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(clds_hazard_pointers_acquire(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_release(IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(clds_hazard_pointers_reclaim(IGNORED_ARG, IGNORED_ARG, IGNORED_ARG)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x1));
    STRICT_EXPECTED_CALL(clds_sorted_list_find_key(IGNORED_ARG, IGNORED_ARG, (void*)0x1));
    STRICT_EXPECTED_CALL(clds_sorted_list_node_release(IGNORED_ARG));
    STRICT_EXPECTED_CALL(clds_sorted_list_set_value(IGNORED_ARG, IGNORED_ARG, (void*)0x1, (CLDS_SORTED_LIST_ITEM*)item_3, NULL, NULL, IGNORED_ARG, NULL, true))
        .SetReturn(CLDS_SORTED_LIST_SET_VALUE_ERROR);

    // act
    result = clds_hash_table_set_value(hash_table, hazard_pointers_thread, (void*)0x1, item_3, NULL, NULL, &old_item, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_SET_VALUE_RESULT, CLDS_HASH_TABLE_SET_VALUE_ERROR, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
    CLDS_HASH_TABLE_NODE_RELEASE(TEST_ITEM, item_3);
}

/* on_sorted_list_skipped_seq_no */

/* Tests_SRS_CLDS_HASH_TABLE_01_075: [ on_sorted_list_skipped_seq_no called with NULL context shall return. ]*/
TEST_FUNCTION(on_sorted_list_skipped_seq_no_with_NULL_returns)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    volatile_atomic int64_t start_seq_no;
    (void)interlocked_exchange_64(&start_seq_no, 0);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, &start_seq_no, test_skipped_seq_no_cb, NULL);
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    SORTED_LIST_SKIPPED_SEQ_NO_CB test_on_sorted_list_skipped_seq_no;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x1));
    STRICT_EXPECTED_CALL(clds_sorted_list_create(hazard_pointers, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG))
        .CaptureArgumentValue_skipped_seq_no_cb(&test_on_sorted_list_skipped_seq_no);
    STRICT_EXPECTED_CALL(clds_sorted_list_insert(IGNORED_ARG, IGNORED_ARG, (CLDS_SORTED_LIST_ITEM*)item, NULL));
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL);
    umock_c_reset_all_calls();

    // act
    test_on_sorted_list_skipped_seq_no(NULL, 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_01_076: [ on_sorted_list_skipped_seq_no shall call the skipped sequence number callback passed to clds_hash_table_create and pass the skipped_sequence_no as skipped_sequence_no argument. ]*/
TEST_FUNCTION(on_sorted_list_skipped_seq_no_calls_the_hash_table_skipped_seq_no)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    volatile_atomic int64_t start_seq_no;
    (void)interlocked_exchange_64(&start_seq_no, 0);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, &start_seq_no, test_skipped_seq_no_cb, NULL);
    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    SORTED_LIST_SKIPPED_SEQ_NO_CB test_on_sorted_list_skipped_seq_no;
    void* test_on_sorted_list_skipped_seq_no_context;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_compute_hash((void*)0x1));
    STRICT_EXPECTED_CALL(clds_sorted_list_create(hazard_pointers, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG))
        .CaptureArgumentValue_skipped_seq_no_cb(&test_on_sorted_list_skipped_seq_no)
        .CaptureArgumentValue_skipped_seq_no_cb_context(&test_on_sorted_list_skipped_seq_no_context);
    STRICT_EXPECTED_CALL(clds_sorted_list_insert(IGNORED_ARG, IGNORED_ARG, (CLDS_SORTED_LIST_ITEM*)item, NULL));
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_skipped_seq_no_cb(NULL, 1));

    // act
    test_on_sorted_list_skipped_seq_no(test_on_sorted_list_skipped_seq_no_context, 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* clds_hash_table_snapshot */

/* Tests_SRS_CLDS_HASH_TABLE_42_013: [ If clds_hash_table is NULL then clds_hash_table_snapshot shall fail and return CLDS_HASH_TABLE_SNAPSHOT_ERROR. ]*/
TEST_FUNCTION(clds_hash_table_snapshot_with_null_clds_hash_table_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    umock_c_reset_all_calls();

    CLDS_HASH_TABLE_ITEM** items;
    uint64_t item_count;

    // act
    CLDS_HASH_TABLE_SNAPSHOT_RESULT result = clds_hash_table_snapshot(NULL, hazard_pointers_thread, &items, &item_count);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_SNAPSHOT_RESULT, CLDS_HASH_TABLE_SNAPSHOT_ERROR, result);

    // cleanup
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_42_014: [ If clds_hazard_pointers_thread is NULL then clds_hash_table_snapshot shall fail and return CLDS_HASH_TABLE_SNAPSHOT_ERROR. ]*/
TEST_FUNCTION(clds_hash_table_snapshot_with_null_clds_hazard_pointers_thread_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HASH_TABLE_HANDLE hash_table;
    volatile_atomic int64_t start_seq_no;
    (void)interlocked_exchange_64(&start_seq_no, 0);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, &start_seq_no, test_skipped_seq_no_cb, NULL);
    umock_c_reset_all_calls();

    CLDS_HASH_TABLE_ITEM** items;
    uint64_t item_count;

    // act
    CLDS_HASH_TABLE_SNAPSHOT_RESULT result = clds_hash_table_snapshot(hash_table, NULL, &items, &item_count);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_SNAPSHOT_RESULT, CLDS_HASH_TABLE_SNAPSHOT_ERROR, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_42_015: [ If items is NULL then clds_hash_table_snapshot shall fail and return CLDS_HASH_TABLE_SNAPSHOT_ERROR. ]*/
TEST_FUNCTION(clds_hash_table_snapshot_with_null_items_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    volatile_atomic int64_t start_seq_no;
    (void)interlocked_exchange_64(&start_seq_no, 0);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, &start_seq_no, test_skipped_seq_no_cb, NULL);
    umock_c_reset_all_calls();

    uint64_t item_count;

    // act
    CLDS_HASH_TABLE_SNAPSHOT_RESULT result = clds_hash_table_snapshot(hash_table, hazard_pointers_thread, NULL, &item_count);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_SNAPSHOT_RESULT, CLDS_HASH_TABLE_SNAPSHOT_ERROR, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_42_016: [ If item_count is NULL then clds_hash_table_snapshot shall fail and return CLDS_HASH_TABLE_SNAPSHOT_ERROR. ]*/
TEST_FUNCTION(clds_hash_table_snapshot_with_null_item_count_fails)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    volatile_atomic int64_t start_seq_no;
    (void)interlocked_exchange_64(&start_seq_no, 0);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, &start_seq_no, test_skipped_seq_no_cb, NULL);
    umock_c_reset_all_calls();

    CLDS_HASH_TABLE_ITEM** items;

    // act
    CLDS_HASH_TABLE_SNAPSHOT_RESULT result = clds_hash_table_snapshot(hash_table, hazard_pointers_thread, &items, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_SNAPSHOT_RESULT, CLDS_HASH_TABLE_SNAPSHOT_ERROR, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_42_019: [ For each bucket in the array: ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_020: [ clds_hash_table_snapshot shall call clds_sorted_list_lock_writes. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_021: [ clds_hash_table_snapshot shall call clds_sorted_list_get_count and add to the running total. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_064: [ If there are no items then clds_hash_table_snapshot shall set items to NULL and item_count to 0 and return CLDS_HASH_TABLE_SNAPSHOT_OK. ]. ]*/
TEST_FUNCTION(clds_hash_table_snapshot_with_empty_table_succeeds)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    volatile_atomic int64_t start_seq_no;
    (void)interlocked_exchange_64(&start_seq_no, 0);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, &start_seq_no, test_skipped_seq_no_cb, NULL);
    umock_c_reset_all_calls();

    CLDS_HASH_TABLE_ITEM** items;
    uint64_t item_count;

    // act
    CLDS_HASH_TABLE_SNAPSHOT_RESULT result = clds_hash_table_snapshot(hash_table, hazard_pointers_thread, &items, &item_count);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_SNAPSHOT_RESULT, CLDS_HASH_TABLE_SNAPSHOT_OK, result);

    ASSERT_ARE_EQUAL(uint64_t, 0, item_count);
    ASSERT_IS_NULL(items);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_42_019: [ For each bucket in the array: ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_020: [ clds_hash_table_snapshot shall call clds_sorted_list_lock_writes. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_021: [ clds_hash_table_snapshot shall call clds_sorted_list_get_count and add to the running total. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_023: [ clds_hash_table_snapshot shall allocate an array of CLDS_HASH_TABLE_ITEM* ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_024: [ For each bucket in the array: ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_025: [ clds_hash_table_snapshot shall call clds_sorted_list_get_count. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_026: [ clds_hash_table_snapshot shall call clds_sorted_list_get_all with the next portion of the allocated array. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_027: [ clds_hash_table_snapshot shall call clds_sorted_list_unlock_writes. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_028: [ clds_hash_table_snapshot shall store the allocated array of items in items. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_029: [ clds_hash_table_snapshot shall store the count of items in item_count. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_031: [ clds_hash_table_snapshot shall succeed and return CLDS_HASH_TABLE_SNAPSHOT_OK. ]*/
TEST_FUNCTION(clds_hash_table_snapshot_with_1_item_succeeds)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    volatile_atomic int64_t start_seq_no;
    (void)interlocked_exchange_64(&start_seq_no, 0);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, &start_seq_no, test_skipped_seq_no_cb, NULL);

    CLDS_HASH_TABLE_ITEM* item = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item, NULL);
    umock_c_reset_all_calls();

    CLDS_HASH_TABLE_ITEM** items;
    uint64_t item_count;

    STRICT_EXPECTED_CALL(clds_sorted_list_lock_writes(IGNORED_ARG));
    STRICT_EXPECTED_CALL(clds_sorted_list_get_count(IGNORED_ARG, hazard_pointers_thread, IGNORED_ARG));

    STRICT_EXPECTED_CALL(malloc_2(IGNORED_ARG, sizeof(CLDS_SORTED_LIST_ITEM*)));

    STRICT_EXPECTED_CALL(clds_sorted_list_get_count(IGNORED_ARG, hazard_pointers_thread, IGNORED_ARG));
    STRICT_EXPECTED_CALL(clds_sorted_list_get_all(IGNORED_ARG, hazard_pointers_thread, IGNORED_ARG, IGNORED_ARG));
    STRICT_EXPECTED_CALL(clds_sorted_list_unlock_writes(IGNORED_ARG));

    // act
    CLDS_HASH_TABLE_SNAPSHOT_RESULT result = clds_hash_table_snapshot(hash_table, hazard_pointers_thread, &items, &item_count);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_SNAPSHOT_RESULT, CLDS_HASH_TABLE_SNAPSHOT_OK, result);

    ASSERT_ARE_EQUAL(uint64_t, 1, item_count);
    ASSERT_IS_NOT_NULL(items);

    ASSERT_ARE_EQUAL(void_ptr, (void*)item, (void*)items[0]);

    // cleanup
    for (uint64_t i = 0; i < item_count; i++)
    {
        CLDS_HASH_TABLE_NODE_RELEASE(TEST_ITEM, items[i]);
    }
    free(items);

    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_42_019: [ For each bucket in the array: ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_020: [ clds_hash_table_snapshot shall call clds_sorted_list_lock_writes. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_021: [ clds_hash_table_snapshot shall call clds_sorted_list_get_count and add to the running total. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_023: [ clds_hash_table_snapshot shall allocate an array of CLDS_HASH_TABLE_ITEM* ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_024: [ For each bucket in the array: ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_025: [ clds_hash_table_snapshot shall call clds_sorted_list_get_count. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_026: [ clds_hash_table_snapshot shall call clds_sorted_list_get_all with the next portion of the allocated array. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_027: [ clds_hash_table_snapshot shall call clds_sorted_list_unlock_writes. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_028: [ clds_hash_table_snapshot shall store the allocated array of items in items. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_029: [ clds_hash_table_snapshot shall store the count of items in item_count. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_031: [ clds_hash_table_snapshot shall succeed and return CLDS_HASH_TABLE_SNAPSHOT_OK. ]*/
TEST_FUNCTION(clds_hash_table_snapshot_with_10_items_same_bucket_succeeds)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    volatile_atomic int64_t start_seq_no;
    (void)interlocked_exchange_64(&start_seq_no, 0);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 20, hazard_pointers, &start_seq_no, test_skipped_seq_no_cb, NULL);

    CLDS_HASH_TABLE_ITEM* original_items[10];
    bool found_originals[10];
    uint32_t number_of_items = 10;

    for (uint32_t i = 0; i < number_of_items; i++)
    {
        original_items[i] = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)(uintptr_t)(0x4242 + i));
        (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)(uintptr_t)((20 * i) + 1), original_items[i], NULL);
        found_originals[i] = false;
    }
    umock_c_reset_all_calls();

    CLDS_HASH_TABLE_ITEM** items;
    uint64_t item_count;

    STRICT_EXPECTED_CALL(clds_sorted_list_lock_writes(IGNORED_ARG));
    STRICT_EXPECTED_CALL(clds_sorted_list_get_count(IGNORED_ARG, hazard_pointers_thread, IGNORED_ARG));

    STRICT_EXPECTED_CALL(malloc_2(IGNORED_ARG, sizeof(CLDS_SORTED_LIST_ITEM*)));

    STRICT_EXPECTED_CALL(clds_sorted_list_get_count(IGNORED_ARG, hazard_pointers_thread, IGNORED_ARG));
    STRICT_EXPECTED_CALL(clds_sorted_list_get_all(IGNORED_ARG, hazard_pointers_thread, IGNORED_ARG, IGNORED_ARG));
    STRICT_EXPECTED_CALL(clds_sorted_list_unlock_writes(IGNORED_ARG));

    // act
    CLDS_HASH_TABLE_SNAPSHOT_RESULT result = clds_hash_table_snapshot(hash_table, hazard_pointers_thread, &items, &item_count);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_SNAPSHOT_RESULT, CLDS_HASH_TABLE_SNAPSHOT_OK, result);

    ASSERT_ARE_EQUAL(uint64_t, number_of_items, item_count);
    ASSERT_IS_NOT_NULL(items);

    for (uint32_t i = 0; i < number_of_items; i++)
    {
        uint32_t original;
        for (original = 0; original < 10; original++)
        {
            if (!found_originals[original] &&
                (void*)original_items[original] == (void*)items[i])
            {
                found_originals[original] = true;
                break;
            }
        }

        if (original >= number_of_items)
        {
            ASSERT_FAIL("The returned item in index %" PRIu32 " was not found in the original", i);
        }
    }

    for (uint32_t i = 0; i < number_of_items; i++)
    {
        ASSERT_IS_TRUE(found_originals[i], "should have found the item with index %" PRIu32, i);
    }

    // cleanup
    for (uint64_t i = 0; i < item_count; i++)
    {
        CLDS_HASH_TABLE_NODE_RELEASE(TEST_ITEM, items[i]);
    }
    free(items);

    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_42_019: [ For each bucket in the array: ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_020: [ clds_hash_table_snapshot shall call clds_sorted_list_lock_writes. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_021: [ clds_hash_table_snapshot shall call clds_sorted_list_get_count and add to the running total. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_023: [ clds_hash_table_snapshot shall allocate an array of CLDS_HASH_TABLE_ITEM* ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_024: [ For each bucket in the array: ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_025: [ clds_hash_table_snapshot shall call clds_sorted_list_get_count. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_026: [ clds_hash_table_snapshot shall call clds_sorted_list_get_all with the next portion of the allocated array. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_027: [ clds_hash_table_snapshot shall call clds_sorted_list_unlock_writes. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_028: [ clds_hash_table_snapshot shall store the allocated array of items in items. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_029: [ clds_hash_table_snapshot shall store the count of items in item_count. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_031: [ clds_hash_table_snapshot shall succeed and return CLDS_HASH_TABLE_SNAPSHOT_OK. ]*/
TEST_FUNCTION(clds_hash_table_snapshot_with_10_items_multiple_buckets_succeeds)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    volatile_atomic int64_t start_seq_no;
    (void)interlocked_exchange_64(&start_seq_no, 0);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, &start_seq_no, test_skipped_seq_no_cb, NULL);

    CLDS_HASH_TABLE_ITEM* original_items[10];
    bool found_originals[10];
    uint32_t number_of_items = 10;

    const uint32_t number_of_sorted_lists = 10;

    for (uint32_t i = 0; i < number_of_items; i++)
    {
        original_items[i] = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)(uintptr_t)(0x4242 + i));
        (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)(uintptr_t)(0x1 + i), original_items[i], NULL);
        found_originals[i] = false;
    }
    umock_c_reset_all_calls();

    CLDS_HASH_TABLE_ITEM** items;
    uint64_t item_count;

    for (uint32_t i = 0; i < number_of_sorted_lists; i++)
    {
        STRICT_EXPECTED_CALL(clds_sorted_list_lock_writes(IGNORED_ARG));
        STRICT_EXPECTED_CALL(clds_sorted_list_get_count(IGNORED_ARG, hazard_pointers_thread, IGNORED_ARG));
    }

    STRICT_EXPECTED_CALL(malloc_2(IGNORED_ARG, sizeof(CLDS_SORTED_LIST_ITEM*)));

    for (uint32_t i = 0; i < number_of_sorted_lists; i++)
    {
        STRICT_EXPECTED_CALL(clds_sorted_list_get_count(IGNORED_ARG, hazard_pointers_thread, IGNORED_ARG));
        STRICT_EXPECTED_CALL(clds_sorted_list_get_all(IGNORED_ARG, hazard_pointers_thread, IGNORED_ARG, IGNORED_ARG));
        STRICT_EXPECTED_CALL(clds_sorted_list_unlock_writes(IGNORED_ARG));
    }

    // act
    CLDS_HASH_TABLE_SNAPSHOT_RESULT result = clds_hash_table_snapshot(hash_table, hazard_pointers_thread, &items, &item_count);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_SNAPSHOT_RESULT, CLDS_HASH_TABLE_SNAPSHOT_OK, result);

    ASSERT_ARE_EQUAL(uint64_t, number_of_items, item_count);
    ASSERT_IS_NOT_NULL(items);

    for (uint32_t i = 0; i < number_of_items; i++)
    {
        uint32_t original;
        for (original = 0; original < number_of_items; original++)
        {
            if (!found_originals[original] &&
                (void*)original_items[original] == (void*)items[i])
            {
                found_originals[original] = true;
                break;
            }
        }

        if (original >= number_of_items)
        {
            ASSERT_FAIL("The returned item in index %" PRIu32 " was not found in the original", i);
        }
    }

    for (uint32_t i = 0; i < number_of_items; i++)
    {
        ASSERT_IS_TRUE(found_originals[i], "should have found the item with index %" PRIu32, i);
    }

    // cleanup
    for (uint64_t i = 0; i < item_count; i++)
    {
        CLDS_HASH_TABLE_NODE_RELEASE(TEST_ITEM, items[i]);
    }
    free(items);

    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_42_019: [ For each bucket in the array: ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_020: [ clds_hash_table_snapshot shall call clds_sorted_list_lock_writes. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_021: [ clds_hash_table_snapshot shall call clds_sorted_list_get_count and add to the running total. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_023: [ clds_hash_table_snapshot shall allocate an array of CLDS_HASH_TABLE_ITEM* ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_024: [ For each bucket in the array: ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_025: [ clds_hash_table_snapshot shall call clds_sorted_list_get_count. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_026: [ clds_hash_table_snapshot shall call clds_sorted_list_get_all with the next portion of the allocated array. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_027: [ clds_hash_table_snapshot shall call clds_sorted_list_unlock_writes. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_028: [ clds_hash_table_snapshot shall store the allocated array of items in items. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_029: [ clds_hash_table_snapshot shall store the count of items in item_count. ]*/
/* Tests_SRS_CLDS_HASH_TABLE_42_031: [ clds_hash_table_snapshot shall succeed and return CLDS_HASH_TABLE_SNAPSHOT_OK. ]*/
TEST_FUNCTION(clds_hash_table_snapshot_with_100_items_multiple_buckets_different_hashes_succeeds)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    volatile_atomic int64_t start_seq_no;
    (void)interlocked_exchange_64(&start_seq_no, 0);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, &start_seq_no, test_skipped_seq_no_cb, NULL);

    CLDS_HASH_TABLE_ITEM* original_items[100];
    bool found_originals[100];
    uint32_t number_of_items = 100;

    const uint32_t number_of_sorted_lists = 100;

    for (uint32_t i = 0; i < number_of_items; i++)
    {
        original_items[i] = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)(uintptr_t)(0x4242 + i));

        STRICT_EXPECTED_CALL(test_compute_hash((void*)(uintptr_t)(0x1 + i)))
            .SetReturn(i);
        (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)(uintptr_t)(0x1 + i), original_items[i], NULL);
        found_originals[i] = false;
        umock_c_reset_all_calls();
    }
    umock_c_reset_all_calls();

    CLDS_HASH_TABLE_ITEM** items;
    uint64_t item_count;

    for (uint32_t i = 0; i < number_of_sorted_lists; i++)
    {
        STRICT_EXPECTED_CALL(clds_sorted_list_lock_writes(IGNORED_ARG));
        STRICT_EXPECTED_CALL(clds_sorted_list_get_count(IGNORED_ARG, hazard_pointers_thread, IGNORED_ARG));
    }

    STRICT_EXPECTED_CALL(malloc_2(IGNORED_ARG, sizeof(CLDS_SORTED_LIST_ITEM*)));

    for (uint32_t i = 0; i < number_of_sorted_lists; i++)
    {
        STRICT_EXPECTED_CALL(clds_sorted_list_get_count(IGNORED_ARG, hazard_pointers_thread, IGNORED_ARG));
        STRICT_EXPECTED_CALL(clds_sorted_list_get_all(IGNORED_ARG, hazard_pointers_thread, IGNORED_ARG, IGNORED_ARG));
        STRICT_EXPECTED_CALL(clds_sorted_list_unlock_writes(IGNORED_ARG));
    }

    // act
    CLDS_HASH_TABLE_SNAPSHOT_RESULT result = clds_hash_table_snapshot(hash_table, hazard_pointers_thread, &items, &item_count);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_SNAPSHOT_RESULT, CLDS_HASH_TABLE_SNAPSHOT_OK, result);

    ASSERT_ARE_EQUAL(uint64_t, number_of_items, item_count);
    ASSERT_IS_NOT_NULL(items);

    for (uint32_t i = 0; i < number_of_items; i++)
    {
        uint32_t original;
        for (original = 0; original < number_of_items; original++)
        {
            if (!found_originals[original] &&
                (void*)original_items[original] == (void*)items[i])
            {
                found_originals[original] = true;
                break;
            }
        }

        if (original >= number_of_items)
        {
            ASSERT_FAIL("The returned item in index %" PRIu32 " was not found in the original", i);
        }
    }

    for (uint32_t i = 0; i < number_of_items; i++)
    {
        ASSERT_IS_TRUE(found_originals[i], "should have found the item with index %" PRIu32, i);
    }

    // cleanup
    for (uint64_t i = 0; i < item_count; i++)
    {
        CLDS_HASH_TABLE_NODE_RELEASE(TEST_ITEM, items[i]);
    }
    free(items);

    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_42_061: [ If there are any other failures then clds_hash_table_snapshot shall fail and return CLDS_HASH_TABLE_SNAPSHOT_ERROR. ]*/
TEST_FUNCTION(clds_hash_table_snapshot_with_10_items_same_bucket_fails_when_underlying_functions_fail)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    volatile_atomic int64_t start_seq_no;
    (void)interlocked_exchange_64(&start_seq_no, 0);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 20, hazard_pointers, &start_seq_no, test_skipped_seq_no_cb, NULL);

    CLDS_HASH_TABLE_ITEM* original_items[10];

    for (uint32_t i = 0; i < 10; i++)
    {
        original_items[i] = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)(uintptr_t)(0x4242 + i));
        (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)(uintptr_t)(0x1 + (20 * i)), original_items[i], NULL);
    }
    umock_c_reset_all_calls();

    CLDS_HASH_TABLE_ITEM** items;
    uint64_t item_count;

    STRICT_EXPECTED_CALL(clds_sorted_list_lock_writes(IGNORED_ARG));
    STRICT_EXPECTED_CALL(clds_sorted_list_get_count(IGNORED_ARG, hazard_pointers_thread, IGNORED_ARG));

    STRICT_EXPECTED_CALL(malloc_2(IGNORED_ARG, sizeof(CLDS_SORTED_LIST_ITEM*)));

    STRICT_EXPECTED_CALL(clds_sorted_list_get_count(IGNORED_ARG, hazard_pointers_thread, IGNORED_ARG));
    STRICT_EXPECTED_CALL(clds_sorted_list_get_all(IGNORED_ARG, hazard_pointers_thread, IGNORED_ARG, IGNORED_ARG));
    STRICT_EXPECTED_CALL(clds_sorted_list_unlock_writes(IGNORED_ARG));

    umock_c_negative_tests_snapshot();

    for (size_t i = 0; i < umock_c_negative_tests_call_count(); i++)
    {
        if (umock_c_negative_tests_can_call_fail(i))
        {
            umock_c_negative_tests_reset();
            umock_c_negative_tests_fail_call(i);

            // act
            CLDS_HASH_TABLE_SNAPSHOT_RESULT result = clds_hash_table_snapshot(hash_table, hazard_pointers_thread, &items, &item_count);

            // assert
            ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_SNAPSHOT_RESULT, CLDS_HASH_TABLE_SNAPSHOT_ERROR, result, "On failed call %zu", i);
        }
    }

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_42_061: [ If there are any other failures then clds_hash_table_snapshot shall fail and return CLDS_HASH_TABLE_SNAPSHOT_ERROR. ]*/
TEST_FUNCTION(clds_hash_table_snapshot_with_10_items_multiple_buckets_fails_when_underlying_functions_fail)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    volatile_atomic int64_t start_seq_no;
    (void)interlocked_exchange_64(&start_seq_no, 0);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, &start_seq_no, test_skipped_seq_no_cb, NULL);

    CLDS_HASH_TABLE_ITEM* original_items[10];

    const uint32_t number_of_sorted_lists = 10;

    for (uint32_t i = 0; i < 10; i++)
    {
        original_items[i] = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)(uintptr_t)(0x4242 + i));
        (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)(uintptr_t)(0x1 + i), original_items[i], NULL);
    }
    umock_c_reset_all_calls();

    CLDS_HASH_TABLE_ITEM** items;
    uint64_t item_count;

    for (uint32_t i = 0; i < number_of_sorted_lists; i++)
    {
        STRICT_EXPECTED_CALL(clds_sorted_list_lock_writes(IGNORED_ARG));
        STRICT_EXPECTED_CALL(clds_sorted_list_get_count(IGNORED_ARG, hazard_pointers_thread, IGNORED_ARG));
    }

    STRICT_EXPECTED_CALL(malloc_2(IGNORED_ARG, sizeof(CLDS_SORTED_LIST_ITEM*)));

    for (uint32_t i = 0; i < number_of_sorted_lists; i++)
    {
        STRICT_EXPECTED_CALL(clds_sorted_list_get_count(IGNORED_ARG, hazard_pointers_thread, IGNORED_ARG));
        STRICT_EXPECTED_CALL(clds_sorted_list_get_all(IGNORED_ARG, hazard_pointers_thread, IGNORED_ARG, IGNORED_ARG));
        STRICT_EXPECTED_CALL(clds_sorted_list_unlock_writes(IGNORED_ARG));
    }

    umock_c_negative_tests_snapshot();

    for (size_t i = 0; i < umock_c_negative_tests_call_count(); i++)
    {
        if (umock_c_negative_tests_can_call_fail(i))
        {
            umock_c_negative_tests_reset();
            umock_c_negative_tests_fail_call(i);

            // act
            CLDS_HASH_TABLE_SNAPSHOT_RESULT result = clds_hash_table_snapshot(hash_table, hazard_pointers_thread, &items, &item_count);

            // assert
            ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_SNAPSHOT_RESULT, CLDS_HASH_TABLE_SNAPSHOT_ERROR, result, "On failed call %zu", i);
        }
    }

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_42_061: [ If there are any other failures then clds_hash_table_snapshot shall fail and return CLDS_HASH_TABLE_SNAPSHOT_ERROR. ]*/
TEST_FUNCTION(clds_hash_table_snapshot_with_20_items_multiple_buckets_different_hashes_fails_when_underlying_functions_fail)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    volatile_atomic int64_t start_seq_no;
    (void)interlocked_exchange_64(&start_seq_no, 0);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 2, hazard_pointers, &start_seq_no, test_skipped_seq_no_cb, NULL);

    CLDS_HASH_TABLE_ITEM* original_items[20];
    uint32_t number_of_items = 20;

    const uint32_t number_of_sorted_lists = 20;

    for (uint32_t i = 0; i < number_of_items; i++)
    {
        original_items[i] = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)(uintptr_t)(0x4242 + i));

        STRICT_EXPECTED_CALL(test_compute_hash((void*)(uintptr_t)(0x1 + i)))
            .SetReturn(i);
        (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)(uintptr_t)(0x1 + i), original_items[i], NULL);
        umock_c_reset_all_calls();
    }
    umock_c_reset_all_calls();

    CLDS_HASH_TABLE_ITEM** items;
    uint64_t item_count;

    for (uint32_t i = 0; i < number_of_sorted_lists; i++)
    {
        STRICT_EXPECTED_CALL(clds_sorted_list_lock_writes(IGNORED_ARG));
        STRICT_EXPECTED_CALL(clds_sorted_list_get_count(IGNORED_ARG, hazard_pointers_thread, IGNORED_ARG));
    }

    STRICT_EXPECTED_CALL(malloc_2(IGNORED_ARG, sizeof(CLDS_SORTED_LIST_ITEM*)));

    for (uint32_t i = 0; i < number_of_sorted_lists; i++)
    {
        STRICT_EXPECTED_CALL(clds_sorted_list_get_count(IGNORED_ARG, hazard_pointers_thread, IGNORED_ARG));
        STRICT_EXPECTED_CALL(clds_sorted_list_get_all(IGNORED_ARG, hazard_pointers_thread, IGNORED_ARG, IGNORED_ARG));
        STRICT_EXPECTED_CALL(clds_sorted_list_unlock_writes(IGNORED_ARG));
    }

    umock_c_negative_tests_snapshot();

    for (size_t i = 0; i < umock_c_negative_tests_call_count(); i++)
    {
        if (umock_c_negative_tests_can_call_fail(i))
        {
            umock_c_negative_tests_reset();
            umock_c_negative_tests_fail_call(i);

            // act
            CLDS_HASH_TABLE_SNAPSHOT_RESULT result = clds_hash_table_snapshot(hash_table, hazard_pointers_thread, &items, &item_count);

            // assert
            ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_SNAPSHOT_RESULT, CLDS_HASH_TABLE_SNAPSHOT_ERROR, result, "On failed call %zu", i);
        }
    }

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

/* Tests_SRS_CLDS_HASH_TABLE_42_022: [ If the addition of the list count causes overflow then clds_hash_table_snapshot shall fail and return CLDS_HASH_TABLE_SNAPSHOT_ERROR. ]*/
TEST_FUNCTION(clds_hash_table_snapshot_fails_if_number_of_items_would_overflow)
{
    // arrange
    CLDS_HAZARD_POINTERS_HANDLE hazard_pointers = clds_hazard_pointers_create();
    CLDS_HAZARD_POINTERS_THREAD_HANDLE hazard_pointers_thread = clds_hazard_pointers_register_thread(hazard_pointers);
    CLDS_HASH_TABLE_HANDLE hash_table;
    volatile_atomic int64_t start_seq_no;
    (void)interlocked_exchange_64(&start_seq_no, 0);
    hash_table = clds_hash_table_create(test_compute_hash, test_key_compare_func, 1, hazard_pointers, &start_seq_no, test_skipped_seq_no_cb, NULL);

    CLDS_HASH_TABLE_ITEM* item_1 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4242);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x1, item_1, NULL);

    CLDS_HASH_TABLE_ITEM* item_2 = CLDS_HASH_TABLE_NODE_CREATE(TEST_ITEM, test_item_cleanup_func, (void*)0x4243);
    (void)clds_hash_table_insert(hash_table, hazard_pointers_thread, (void*)0x2, item_2, NULL);

    umock_c_reset_all_calls();

    CLDS_HASH_TABLE_ITEM** items;
    uint64_t item_count;

    uint64_t mocked_item_count_1 = UINT64_MAX / 2 + 1;
    uint64_t mocked_item_count_2 = UINT64_MAX / 2 + 1;

    STRICT_EXPECTED_CALL(clds_sorted_list_lock_writes(IGNORED_ARG));
    STRICT_EXPECTED_CALL(clds_sorted_list_get_count(IGNORED_ARG, hazard_pointers_thread, IGNORED_ARG))
        .CopyOutArgumentBuffer_item_count(&mocked_item_count_1, sizeof(mocked_item_count_1));

    STRICT_EXPECTED_CALL(clds_sorted_list_lock_writes(IGNORED_ARG));
    STRICT_EXPECTED_CALL(clds_sorted_list_get_count(IGNORED_ARG, hazard_pointers_thread, IGNORED_ARG))
        .CopyOutArgumentBuffer_item_count(&mocked_item_count_2, sizeof(mocked_item_count_2));

    STRICT_EXPECTED_CALL(clds_sorted_list_unlock_writes(IGNORED_ARG));
    STRICT_EXPECTED_CALL(clds_sorted_list_unlock_writes(IGNORED_ARG));

    // act
    CLDS_HASH_TABLE_SNAPSHOT_RESULT result = clds_hash_table_snapshot(hash_table, hazard_pointers_thread, &items, &item_count);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(CLDS_HASH_TABLE_SNAPSHOT_RESULT, CLDS_HASH_TABLE_SNAPSHOT_ERROR, result);

    // cleanup
    clds_hash_table_destroy(hash_table);
    clds_hazard_pointers_destroy(hazard_pointers);
}

END_TEST_SUITE(TEST_SUITE_NAME_FROM_CMAKE)
