int wc_RsaPad_ex(const byte* input, word32 inputLen, byte* pkcsBlock,
    word32 pkcsBlockLen, byte padValue, WC_RNG* rng, int padType,
    enum wc_HashType hType, int mgf, byte* optionalLabel, word32 labelLen,
    int saltLen, int bits, void* heap)
{
    int ret;

    switch (padType)
    {
        case WC_RSA_PKCSV15_PAD:
            /*WOLFSSL_MSG("wolfSSL Using RSA PKCSV15 padding");*/
            ret = RsaPad(input, inputLen, pkcsBlock, pkcsBlockLen,
                                                                 padValue, rng);
            break;

#ifndef WC_NO_RNG
    #ifndef WC_NO_RSA_OAEP
        case WC_RSA_OAEP_PAD:
            WOLFSSL_MSG("wolfSSL Using RSA OAEP padding");
            ret = RsaPad_OAEP(input, inputLen, pkcsBlock, pkcsBlockLen,
                           padValue, rng, hType, mgf, optionalLabel, labelLen, heap);
            break;
    #endif

    #ifdef WC_RSA_PSS
        case WC_RSA_PSS_PAD:
            WOLFSSL_MSG("wolfSSL Using RSA PSS padding");
            ret = RsaPad_PSS(input, inputLen, pkcsBlock, pkcsBlockLen, rng,
                                               hType, mgf, saltLen, bits, heap);
            break;
    #endif
#endif /* !WC_NO_RNG */

    #ifdef WC_RSA_NO_PADDING
        case WC_RSA_NO_PAD:
        {
            int bytes = (bits + WOLFSSL_BIT_SIZE - 1) / WOLFSSL_BIT_SIZE;

            WOLFSSL_MSG("wolfSSL Using NO padding");

            /* In the case of no padding being used check that input is exactly
             * the RSA key length */
            if ((bits <= 0) || (inputLen != (word32)bytes)) {
                WOLFSSL_MSG("Bad input size");
                ret = RSA_PAD_E;
            }
            else {
                XMEMCPY(pkcsBlock, input, inputLen);
                ret = 0;
            }
            break;
        }
    #endif

        default:
            WOLFSSL_MSG("Unknown RSA Pad Type");
            ret = RSA_PAD_E;
    }

    /* silence warning if not used with padding scheme */
    (void)input;
    (void)inputLen;
    (void)pkcsBlock;
    (void)pkcsBlockLen;
    (void)padValue;
    (void)rng;
    (void)padType;
    (void)hType;
    (void)mgf;
    (void)optionalLabel;
    (void)labelLen;
    (void)saltLen;
    (void)bits;
    (void)heap;

    return ret;
}