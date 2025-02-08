int TLSX_Parse(WOLFSSL* ssl, const byte* input, word16 length, byte msgType,
                                                                 Suites *suites)
{
    int ret = 0;
    word16 offset = 0;
    byte isRequest = (msgType == client_hello ||
                      msgType == certificate_request);

#ifdef HAVE_EXTENDED_MASTER
    byte pendingEMS = 0;
#endif
#if defined(WOLFSSL_TLS13) && (defined(HAVE_SESSION_TICKET) || !defined(NO_PSK))
    int pskDone = 0;
#endif

    if (!ssl || !input || (isRequest && !suites))
        return BAD_FUNC_ARG;

    while (ret == 0 && offset < length) {
        word16 type;
        word16 size;

#if defined(WOLFSSL_TLS13) && (defined(HAVE_SESSION_TICKET) || !defined(NO_PSK))
        if (msgType == client_hello && pskDone) {
            WOLFSSL_ERROR_VERBOSE(PSK_KEY_ERROR);
            return PSK_KEY_ERROR;
        }
#endif

        if (length - offset < HELLO_EXT_TYPE_SZ + OPAQUE16_LEN)
            return BUFFER_ERROR;

        ato16(input + offset, &type);
        offset += HELLO_EXT_TYPE_SZ;

        ato16(input + offset, &size);
        offset += OPAQUE16_LEN;

        if (length - offset < size)
            return BUFFER_ERROR;

        switch (type) {
// <MASK>
#ifdef WOLFSSL_SRTP
            case TLSX_USE_SRTP:
                WOLFSSL_MSG("Use SRTP extension received");
                ret = SRTP_PARSE(ssl, input + offset, size, isRequest);
                break;
#endif
            default:
                WOLFSSL_MSG("Unknown TLS extension type");
        }

        /* offset should be updated here! */
        offset += size;
    }

#ifdef HAVE_EXTENDED_MASTER
    if (IsAtLeastTLSv1_3(ssl->version) && msgType == hello_retry_request) {
        /* Don't change EMS status until server_hello received.
         * Second ClientHello must have same extensions.
         */
    }
    else if (!isRequest && ssl->options.haveEMS && !pendingEMS)
        ssl->options.haveEMS = 0;
#endif

    if (ret == 0)
        ret = SNI_VERIFY_PARSE(ssl, isRequest);
    if (ret == 0)
        ret = TCA_VERIFY_PARSE(ssl, isRequest);

    return ret;
}