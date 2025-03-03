/*
 * This file is part of the Pico HSM distribution (https://github.com/polhenarejos/pico-hsm).
 * Copyright (c) 2022 Pol Henarejos.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "crypto_utils.h"
#include "sc_hsm.h"
#include "kek.h"

int cmd_key_unwrap() {
    int key_id = P1(apdu), r = 0;
    if (P2(apdu) != 0x93)
        return SW_WRONG_P1P2();
    if (!isUserAuthenticated)
        return SW_SECURITY_STATUS_NOT_SATISFIED();
    int key_type = dkek_type_key(apdu.data);
    uint8_t kdom = -1, *allowed = NULL;
    size_t allowed_len = 0;
    if (key_type == 0x0)
        return SW_DATA_INVALID();
    if (key_type == HSM_KEY_RSA) {
        mbedtls_rsa_context ctx;
        mbedtls_rsa_init(&ctx);
        do {
            r = dkek_decode_key(++kdom, &ctx, apdu.data, apdu.nc, NULL, &allowed, &allowed_len);
        } while((r == CCID_ERR_FILE_NOT_FOUND || r == CCID_WRONG_DKEK) && kdom < MAX_KEY_DOMAINS);
        if (r != CCID_OK) {
            mbedtls_rsa_free(&ctx);
            return SW_EXEC_ERROR();
        }
        r = store_keys(&ctx, HSM_KEY_RSA, key_id);
        mbedtls_rsa_free(&ctx);
        if (r != CCID_OK) {
            return SW_EXEC_ERROR();
        }
    }
    else if (key_type == HSM_KEY_EC) {
        mbedtls_ecdsa_context ctx;
        mbedtls_ecdsa_init(&ctx);
        do {
            r = dkek_decode_key(++kdom, &ctx, apdu.data, apdu.nc, NULL, &allowed, &allowed_len);
        } while((r == CCID_ERR_FILE_NOT_FOUND || r == CCID_WRONG_DKEK) && kdom < MAX_KEY_DOMAINS);
        if (r != CCID_OK) {
            mbedtls_ecdsa_free(&ctx);
            return SW_EXEC_ERROR();
        }
        r = store_keys(&ctx, HSM_KEY_EC, key_id);
        mbedtls_ecdsa_free(&ctx);
        if (r != CCID_OK) {
            return SW_EXEC_ERROR();
        }
    }
    else if (key_type == HSM_KEY_AES) {
        uint8_t aes_key[32];
        int key_size = 0, aes_type = 0;
        do {
            r = dkek_decode_key(++kdom, aes_key, apdu.data, apdu.nc, &key_size, &allowed, &allowed_len);
        } while((r == CCID_ERR_FILE_NOT_FOUND || r == CCID_WRONG_DKEK) && kdom < MAX_KEY_DOMAINS);
        if (r != CCID_OK) {
            return SW_EXEC_ERROR();
        }
        if (key_size == 32)
            aes_type = HSM_KEY_AES_256;
        else if (key_size == 24)
            aes_type = HSM_KEY_AES_192;
        else if (key_size == 16)
            aes_type = HSM_KEY_AES_128;
        else
            return SW_EXEC_ERROR();
        r = store_keys(aes_key, aes_type, key_id);
        if (r != CCID_OK) {
            return SW_EXEC_ERROR();
        }
    }
    if ((allowed != NULL && allowed_len > 0) || kdom >= 0) {
        size_t meta_len = (allowed_len > 0 ? 2+allowed_len : 0) + (kdom >= 0 ? 3 : 0);
        uint8_t *meta = (uint8_t *)calloc(1,meta_len), *m = meta;
        if (allowed_len > 0) {
            *m++ = 0x91;
            *m++ = allowed_len;
            memcpy(m, allowed, allowed_len); m += allowed_len;
        }
        if (kdom >= 0) {
            *m++ = 0x92;
            *m++ = 1;
            *m++ = kdom;
        }
        r = meta_add((KEY_PREFIX << 8) | key_id, meta, meta_len);
        free(meta);
        if (r != CCID_OK)
            return r;
    }
    return SW_OK();
}
