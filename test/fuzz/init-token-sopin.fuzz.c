/* SPDX-License-Identifier: BSD-2-Clause */
#include "config.h"

#include <stdbool.h>
#include <stdlib.h>

#include "attrs.h"
#include "log.h"
#include "parser.h"
#include "pkcs11.h"

#include "wrap_tpm.h"

static const uint8_t *_data = NULL;
static size_t _size = 0;

static void test(void **state) {
    UNUSED(state);

    /*
     * we just use an in memory db so we don't need to fake sqlite or deal
     *
     * with temp file cleanup
     */
    setenv("TPM2_PKCS11_STORE", ":memory:", 1);

    will_return_always(__wrap_backend_fapi_init, CKR_GENERAL_ERROR);
    will_return_always(__wrap_Esys_Initialize, TSS2_RC_SUCCESS);
    will_return_always(__wrap_Tss2_TctiLdr_Initialize, TSS2_RC_SUCCESS);
    will_return_always(__wrap_Tss2_TctiLdr_Finalize, TSS2_RC_SUCCESS);
    //will_return_always(__wrap_Esys_GetCapability, TSS2_RC_SUCCESS);
    will_return_always(__wrap_Esys_Finalize, TSS2_RC_SUCCESS);
    will_return_always(__wrap_Esys_TR_FromTPMPublic, TSS2_RC_SUCCESS);
    will_return_always(__wrap_Esys_TR_Serialize, TSS2_RC_SUCCESS);
    // will_return_always(__wrap_Esys_TR_Deserialize, TSS2_RC_SUCCESS);
    will_return_always(__wrap_Esys_TR_SetAuth, TSS2_RC_SUCCESS);
    will_return_always(__wrap_Esys_StartAuthSession, TSS2_RC_SUCCESS);
    // will_return_always(__wrap_Esys_TRSess_GetAttributes, TSS2_RC_SUCCESS);
    will_return_always(__wrap_Esys_TRSess_SetAttributes, TSS2_RC_SUCCESS);
    // will_return_always(__wrap_Esys_CreateLoaded, TSS2_RC_SUCCESS);
    will_return_always(__wrap_Esys_Create, TSS2_RC_SUCCESS);
    will_return_always(__wrap_Esys_FlushContext, TSS2_RC_SUCCESS);

    CK_RV rv = C_Initialize(NULL);
    assert_int_equal(rv, CKR_OK);

    CK_SLOT_ID slot_list;
    CK_ULONG count = 1;

    rv = C_GetSlotList(0, &slot_list, &count);
    assert_int_equal(rv, CKR_OK);
    assert_int_not_equal(count, 0);

    CK_BYTE label[32] = "                        my label";

    /* FUZZ TARGET C_InitToken SOPIN */
    rv = C_InitToken(slot_list, (CK_BYTE_PTR)_data, _size, label);
    /* any pin should work */
    assert_int_equal(rv, CKR_OK);

    rv = C_Finalize(NULL);
    assert_int_equal(rv, CKR_OK);
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {

    _data = data;
    _size = size;

    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test),
    };

    cmocka_run_group_tests(tests, NULL, NULL);
    return 0;
}
