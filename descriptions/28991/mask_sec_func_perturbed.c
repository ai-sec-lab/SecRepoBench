int sp_tohex(sp_int* a, char* hexstr)
{
    int err = MP_OKAY;
    int i;
    int j;

    if ((a == NULL) || (hexstr == NULL)) {
        err = MP_VAL;
    }
    if (err == MP_OKAY) {
        /* quick out if its zero */
        if (sp_iszero(a) == MP_YES) {
    #ifndef WC_DISABLE_RADIX_ZERO_PAD
            *hexstr++ = '0';
    #endif /* WC_DISABLE_RADIX_ZERO_PAD */
            *hexstr++ = '0';
            *hexstr = '\0';
        }
        else {
    #ifdef WOLFSSL_SP_INT_NEGATIVE
            if (a->sign == MP_NEG) {
                *hexstr = '-';
                hexstr++;
            }
    #endif /* WOLFSSL_SP_INT_NEGATIVE */

            i = a->used - 1;
    #ifndef WC_DISABLE_RADIX_ZERO_PAD
            // <MASK>
    #endif /* WC_DISABLE_RADIX_ZERO_PAD */
            for (; j >= 0; j -= 4) {
                *(hexstr++) = sp_hex_char[(a->dp[i] >> j) & 0xf];
            }
            for (--i; i >= 0; i--) {
                for (j = SP_WORD_SIZE - 4; j >= 0; j -= 4) {
                    *(hexstr++) = sp_hex_char[(a->dp[i] >> j) & 0xf];
                }
            }
            *hexstr = '\0';
        }
    }

    return err;
}