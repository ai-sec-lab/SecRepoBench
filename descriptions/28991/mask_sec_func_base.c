int sp_tohex(sp_int* a, char* str)
{
    int err = MP_OKAY;
    int i;
    int j;

    if ((a == NULL) || (str == NULL)) {
        err = MP_VAL;
    }
    if (err == MP_OKAY) {
        /* quick out if its zero */
        if (sp_iszero(a) == MP_YES) {
    #ifndef WC_DISABLE_RADIX_ZERO_PAD
            *str++ = '0';
    #endif /* WC_DISABLE_RADIX_ZERO_PAD */
            *str++ = '0';
            *str = '\0';
        }
        else {
    #ifdef WOLFSSL_SP_INT_NEGATIVE
            if (a->sign == MP_NEG) {
                *str = '-';
                str++;
            }
    #endif /* WOLFSSL_SP_INT_NEGATIVE */

            i = a->used - 1;
    #ifndef WC_DISABLE_RADIX_ZERO_PAD
            // <MASK>
    #endif /* WC_DISABLE_RADIX_ZERO_PAD */
            for (; j >= 0; j -= 4) {
                *(str++) = sp_hex_char[(a->dp[i] >> j) & 0xf];
            }
            for (--i; i >= 0; i--) {
                for (j = SP_WORD_SIZE - 4; j >= 0; j -= 4) {
                    *(str++) = sp_hex_char[(a->dp[i] >> j) & 0xf];
                }
            }
            *str = '\0';
        }
    }

    return err;
}