case WC_RSA_NO_PAD:
            WOLFSSL_MSG("wolfSSL Using NO padding");

            /* In the case of no padding being used check that input is exactly
             * the RSA key length */
            if (bits <= 0 || inputLen != ((word32)bits/WOLFSSL_BIT_SIZE)) {
                WOLFSSL_MSG("Bad input size");
                ret = RSA_PAD_E;
            }
            else {
                XMEMCPY(pkcsBlock, input, inputLen);
                ret = 0;
            }
            break;