#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include <sapi/marshal.h>
#include <marshal/tss2_endian.h>

/*
 * Success case
 */
static void
tpma_marshal_success(void **state)
{
    TPMA_ALGORITHM alg = {0}, *ptr;
    TPMA_SESSION session = {0}, *ptr2;
    uint8_t buffer[sizeof(alg)] = { 0 };
    size_t  buffer_size = sizeof(buffer);
    uint8_t buffer2[sizeof(session)] = { 0 };
    size_t  buffer_size2 = sizeof(buffer2);
    uint32_t alg_expected = HOST_TO_BE_32(TPMA_ALGORITHM_ASYMMETRIC | TPMA_ALGORITHM_SIGNING);
    uint8_t session_expected = TPMA_SESSION_AUDIT | TPMA_SESSION_AUDITRESET | TPMA_SESSION_DECRYPT;
    TSS2_RC rc;

    alg.asymmetric = 1;
    alg.signing = 1;
    ptr = (TPMA_ALGORITHM *)buffer;

    rc = TPMA_ALGORITHM_Marshal(alg, buffer, buffer_size, NULL);
    assert_int_equal (rc, TSS2_RC_SUCCESS);
    assert_int_equal (ptr->val, alg_expected);

    session.audit = 1;
    session.decrypt = 1;
    session.auditReset = 1;
    ptr2 = (TPMA_SESSION *)buffer2;

    rc = TPMA_SESSION_Marshal(session, buffer2, buffer_size2, NULL);
    assert_int_equal (rc, TSS2_RC_SUCCESS);
    assert_int_equal (ptr2->val, session_expected);
}

/*
 * Success case with a valid offset
 */
static void
tpma_marshal_success_offset(void **state)
{
    TPMA_ALGORITHM alg = {0}, *ptr;
    TPMA_SESSION session = {0}, *ptr2;
    uint8_t buffer[sizeof(alg) + 10] = { 0 };
    size_t  buffer_size = sizeof(buffer);
    uint8_t buffer2[sizeof(session) + 14] = { 0 };
    size_t  buffer_size2 = sizeof(buffer2);
    size_t offset = 10;
    uint32_t alg_expected = HOST_TO_BE_32(TPMA_ALGORITHM_ASYMMETRIC | TPMA_ALGORITHM_SIGNING);
    uint8_t session_expected = TPMA_SESSION_AUDIT | TPMA_SESSION_AUDITRESET | TPMA_SESSION_DECRYPT;
    TSS2_RC rc;

    alg.asymmetric = 1;
    alg.signing = 1;
    ptr = (TPMA_ALGORITHM *)&buffer[10];

    rc = TPMA_ALGORITHM_Marshal(alg, buffer, buffer_size, &offset);
    assert_int_equal (rc, TSS2_RC_SUCCESS);
    assert_int_equal (ptr->val, alg_expected);
    assert_int_equal (offset, sizeof (buffer));

    session.audit = 1;
    session.decrypt = 1;
    session.auditReset = 1;
    ptr2 = (TPMA_SESSION *)&buffer2[14];

    rc = TPMA_SESSION_Marshal(session, buffer2, buffer_size2, &offset);
    assert_int_equal (rc, TSS2_RC_SUCCESS);
    assert_int_equal (ptr2->val, session_expected);
    assert_int_equal (offset, sizeof (buffer2));
}

/*
 * Success case with a null buffer
 */
static void
tpma_marshal_buffer_null_with_offset(void **state)
{
    TPMA_ALGORITHM alg = {0};
    TPMA_SESSION session = {0};
    size_t offset = 100;
    TSS2_RC rc;

    alg.asymmetric = 1;
    alg.signing = 1;

    rc = TPMA_ALGORITHM_Marshal(alg, NULL, sizeof(alg), &offset);
    assert_int_equal (rc, TSS2_RC_SUCCESS);
    assert_int_equal (offset, 100 + sizeof(alg));

    session.audit = 1;
    session.decrypt = 1;
    session.auditReset = 1;
    offset = 100;

    rc = TPMA_SESSION_Marshal(session, NULL, sizeof(session), &offset);
    assert_int_equal (rc, TSS2_RC_SUCCESS);
    assert_int_equal (offset, 100 + sizeof(session));
}

/*
 * Invalid case with a null buffer and a null offset
 */
static void
tpma_marshal_buffer_null_offset_null(void **state)
{
    TPMA_ALGORITHM alg = {0};
    TPMA_SESSION session = {0};
    TSS2_RC rc;

    alg.asymmetric = 1;
    alg.signing = 1;

    rc = TPMA_ALGORITHM_Marshal(alg, NULL, sizeof(alg), NULL);
    assert_int_equal (rc, TSS2_TYPES_RC_BAD_REFERENCE);

    session.audit = 1;
    session.decrypt = 1;
    session.auditReset = 1;

    rc = TPMA_SESSION_Marshal(session, NULL, sizeof(session), NULL);
    assert_int_equal (rc, TSS2_TYPES_RC_BAD_REFERENCE);
}

/*
 * Invalid case with not big enough buffer
 */
static void
tpma_marshal_buffer_size_lt_data_nad_lt_offset(void **state)
{
    TPMA_ALGORITHM alg = {0};
    TPMA_SESSION session = {0};
    uint8_t buffer[sizeof(alg)] = { 0 };
    size_t  buffer_size = sizeof(buffer);
    uint8_t buffer2[sizeof(session)] = { 0 };
    size_t  buffer_size2 = sizeof(buffer2);
    size_t offset = 2;
    TSS2_RC rc;

    alg.asymmetric = 1;
    alg.signing = 1;

    rc = TPMA_ALGORITHM_Marshal(alg, buffer, buffer_size, &offset);
    assert_int_equal (rc, TSS2_TYPES_RC_INSUFFICIENT_BUFFER);
    assert_int_equal (offset, 2);

    session.audit = 1;
    session.decrypt = 1;
    session.auditReset = 1;

    rc = TPMA_SESSION_Marshal(session, buffer2, buffer_size2, &offset);
    assert_int_equal (rc, TSS2_TYPES_RC_INSUFFICIENT_BUFFER);
    assert_int_equal (offset, 2);
}

/*
 * Success case
 */
static void
tpma_unmarshal_success(void **state)
{
    TPMA_ALGORITHM alg = {0};
    TPMA_SESSION session = {0};
    uint8_t buffer[sizeof(alg) + sizeof(session)] = { 0 };
    size_t buffer_size = sizeof(buffer);
    size_t offset = 0;
    uint32_t alg_expected = HOST_TO_BE_32(TPMA_ALGORITHM_ASYMMETRIC | TPMA_ALGORITHM_SIGNING);
    uint8_t session_expected = TPMA_SESSION_AUDIT | TPMA_SESSION_AUDITRESET | TPMA_SESSION_DECRYPT;
    uint32_t *ptr;
    uint8_t *ptr2;
    TSS2_RC rc;

    ptr = (uint32_t *)buffer;
    ptr2 = (uint8_t *)ptr + 4;

    *ptr = alg_expected;
    *ptr2 = session_expected;

    rc = TPMA_ALGORITHM_Unmarshal(buffer, buffer_size, &offset, &alg);
    assert_int_equal (rc, TSS2_RC_SUCCESS);
    assert_int_equal (alg.val, BE_TO_HOST_32(alg_expected));
    assert_int_equal (offset, 4);


    rc = TPMA_SESSION_Unmarshal(buffer, buffer_size, &offset, &session);
    assert_int_equal (rc, TSS2_RC_SUCCESS);
    assert_int_equal (session.val, session_expected);
    assert_int_equal (offset, 5);
}

/*
 * Invalid test case with buffer null and dest null
 */
static void
tpma_unmarshal_dest_null_buff_null(void **state)
{
    size_t offset = 0;
    TSS2_RC rc;

    rc = TPMA_ALGORITHM_Unmarshal(NULL, 20, &offset, NULL);
    assert_int_equal (rc, TSS2_TYPES_RC_BAD_REFERENCE);
    assert_int_equal (offset, 0);


    rc = TPMA_SESSION_Unmarshal(NULL, 20, &offset, NULL);
    assert_int_equal (rc, TSS2_TYPES_RC_BAD_REFERENCE);
    assert_int_equal (offset, 0);
}

/*
 * Invalid test case with offset null and dest null
 */
static void
tpma_unmarshal_buffer_null_offset_null(void **state)
{
    TPMA_ALGORITHM alg = {0};
    TPMA_SESSION session = {0};
    uint8_t buffer[sizeof(alg) + sizeof(session)] = { 0 };
    size_t buffer_size = sizeof(buffer);
    TSS2_RC rc;

    rc = TPMA_ALGORITHM_Unmarshal(buffer, buffer_size, NULL, NULL);
    assert_int_equal (rc, TSS2_TYPES_RC_BAD_REFERENCE);


    rc = TPMA_SESSION_Unmarshal(buffer, buffer_size, NULL, NULL);
    assert_int_equal (rc, TSS2_TYPES_RC_BAD_REFERENCE);
}
/*
 * Test case ensures the offset is updated when dest is NULL
 * and offset is valid
 */
static void
tpma_unmarshal_dest_null_offset_valid(void **state)
{
    TPMA_ALGORITHM alg = {0};
    TPMA_SESSION session = {0};
    uint8_t buffer[sizeof(alg) + sizeof(session)] = { 0 };
    size_t buffer_size = sizeof(buffer);
    size_t offset = 0;
    uint32_t alg_expected = HOST_TO_BE_32(TPMA_ALGORITHM_ASYMMETRIC | TPMA_ALGORITHM_SIGNING);
    uint8_t session_expected = TPMA_SESSION_AUDIT | TPMA_SESSION_AUDITRESET | TPMA_SESSION_DECRYPT;
    uint32_t *ptr;
    uint8_t *ptr2;
    TSS2_RC rc;

    ptr = (uint32_t *)buffer;
    ptr2 = (uint8_t *)ptr + 4;

    *ptr = alg_expected;
    *ptr2 = session_expected;

    alg.asymmetric = 1;
    alg.signing = 1;

    rc = TPMA_ALGORITHM_Unmarshal(buffer, buffer_size, &offset, NULL);
    assert_int_equal (rc, TSS2_RC_SUCCESS);
    assert_int_equal (offset, sizeof(alg));

    session.audit = 1;
    session.decrypt = 1;
    session.auditReset = 1;

    rc = TPMA_SESSION_Unmarshal(buffer, buffer_size, &offset, NULL);
    assert_int_equal (rc, TSS2_RC_SUCCESS);
    assert_int_equal (offset, sizeof(buffer));
}
/*
 * Invalid case with not big enough buffer
 */
static void
tpma_unmarshal_buffer_size_lt_data_nad_lt_offset(void **state)
{
    TPMA_ALGORITHM alg = {0};
    TPMA_SESSION session = {0};
    uint8_t buffer[sizeof(alg) + sizeof(session)] = { 0 };
    size_t offset = 1;
    TSS2_RC rc;

    alg.asymmetric = 1;
    alg.signing = 1;

    rc = TPMA_ALGORITHM_Unmarshal(buffer, sizeof(alg), &offset, &alg);
    assert_int_equal (rc, TSS2_TYPES_RC_INSUFFICIENT_BUFFER);
    assert_int_equal (offset, 1);

    session.audit = 1;
    session.decrypt = 1;
    session.auditReset = 1;

    rc = TPMA_SESSION_Unmarshal(buffer, 1, &offset, &session);
    assert_int_equal (rc, TSS2_TYPES_RC_INSUFFICIENT_BUFFER);
    assert_int_equal (offset, 1);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test (tpma_marshal_success),
        cmocka_unit_test (tpma_marshal_success_offset),
        cmocka_unit_test (tpma_marshal_buffer_null_with_offset),
        cmocka_unit_test (tpma_marshal_buffer_null_offset_null),
        cmocka_unit_test (tpma_marshal_buffer_size_lt_data_nad_lt_offset),
        cmocka_unit_test (tpma_unmarshal_success),
        cmocka_unit_test (tpma_unmarshal_dest_null_buff_null),
        cmocka_unit_test (tpma_unmarshal_buffer_null_offset_null),
        cmocka_unit_test (tpma_unmarshal_dest_null_offset_valid),
        cmocka_unit_test (tpma_unmarshal_buffer_size_lt_data_nad_lt_offset),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
