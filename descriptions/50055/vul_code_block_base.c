#ifdef HAVE_SNI
            case TLSX_SERVER_NAME:
                WOLFSSL_MSG("SNI extension received");
            #ifdef WOLFSSL_DEBUG_TLS
                WOLFSSL_BUFFER(input + offset, size);
            #endif

#if defined(WOLFSSL_TLS13) && defined(HAVE_SNI)
                if (IsAtLeastTLSv1_3(ssl->version) &&
                        msgType != client_hello &&
                        msgType != encrypted_extensions) {
                    return EXT_NOT_ALLOWED;
                }
                else if (!IsAtLeastTLSv1_3(ssl->version) &&
                         msgType == encrypted_extensions) {
                    return EXT_NOT_ALLOWED;
                }
#endif
                ret = SNI_PARSE(ssl, input + offset, size, isRequest);
                break;
#endif

            case TLSX_TRUSTED_CA_KEYS:
                WOLFSSL_MSG("Trusted CA extension received");
            #ifdef WOLFSSL_DEBUG_TLS
                WOLFSSL_BUFFER(input + offset, size);
            #endif

#if defined(WOLFSSL_TLS13) && defined(HAVE_TRUSTED_CA)
                if (IsAtLeastTLSv1_3(ssl->version) &&
                        msgType != client_hello &&
                        msgType != encrypted_extensions) {
                    return EXT_NOT_ALLOWED;
                }
#endif
                ret = TCA_PARSE(ssl, input + offset, size, isRequest);
                break;

            case TLSX_MAX_FRAGMENT_LENGTH:
                WOLFSSL_MSG("Max Fragment Length extension received");
            #ifdef WOLFSSL_DEBUG_TLS
                WOLFSSL_BUFFER(input + offset, size);
            #endif

#if defined(WOLFSSL_TLS13) && defined(HAVE_MAX_FRAGMENT)
                if (IsAtLeastTLSv1_3(ssl->version) &&
                        msgType != client_hello &&
                        msgType != encrypted_extensions) {
                    WOLFSSL_ERROR_VERBOSE(EXT_NOT_ALLOWED);
                    return EXT_NOT_ALLOWED;
                }
                else if (!IsAtLeastTLSv1_3(ssl->version) &&
                         msgType == encrypted_extensions) {
                    WOLFSSL_ERROR_VERBOSE(EXT_NOT_ALLOWED);
                    return EXT_NOT_ALLOWED;
                }
#endif
                ret = MFL_PARSE(ssl, input + offset, size, isRequest);
                break;

            case TLSX_TRUNCATED_HMAC:
                WOLFSSL_MSG("Truncated HMAC extension received");
            #ifdef WOLFSSL_DEBUG_TLS
                WOLFSSL_BUFFER(input + offset, size);
            #endif

#if defined(WOLFSSL_TLS13) && defined(HAVE_TRUNCATED_HMAC)
                if (IsAtLeastTLSv1_3(ssl->version))
                    break;
#endif
                ret = THM_PARSE(ssl, input + offset, size, isRequest);
                break;

            case TLSX_SUPPORTED_GROUPS:
                WOLFSSL_MSG("Supported Groups extension received");
            #ifdef WOLFSSL_DEBUG_TLS
                WOLFSSL_BUFFER(input + offset, size);
            #endif

#if defined(WOLFSSL_TLS13) && defined(HAVE_SUPPORTED_CURVES)
                if (IsAtLeastTLSv1_3(ssl->version) &&
                        msgType != client_hello &&
                        msgType != encrypted_extensions) {
                    WOLFSSL_ERROR_VERBOSE(EXT_NOT_ALLOWED);
                    return EXT_NOT_ALLOWED;
                }
                else if (!IsAtLeastTLSv1_3(ssl->version) &&
                         msgType == encrypted_extensions) {
                    WOLFSSL_ERROR_VERBOSE(EXT_NOT_ALLOWED);
                    return EXT_NOT_ALLOWED;
                }
#endif
                ret = EC_PARSE(ssl, input + offset, size, isRequest);
                break;

            case TLSX_EC_POINT_FORMATS:
                WOLFSSL_MSG("Point Formats extension received");
            #ifdef WOLFSSL_DEBUG_TLS
                WOLFSSL_BUFFER(input + offset, size);
            #endif

#if defined(WOLFSSL_TLS13) && defined(HAVE_SUPPORTED_CURVES)
                if (IsAtLeastTLSv1_3(ssl->version))
                    break;
#endif
                ret = PF_PARSE(ssl, input + offset, size, isRequest);
                break;

            case TLSX_STATUS_REQUEST:
                WOLFSSL_MSG("Certificate Status Request extension received");
            #ifdef WOLFSSL_DEBUG_TLS
                WOLFSSL_BUFFER(input + offset, size);
            #endif

#if defined(WOLFSSL_TLS13) && defined(HAVE_CERTIFICATE_STATUS_REQUEST)
                if (IsAtLeastTLSv1_3(ssl->version) &&
                        msgType != client_hello &&
                        msgType != certificate_request &&
                        msgType != certificate) {
                     break;
                }
 #endif
                ret = CSR_PARSE(ssl, input + offset, size, isRequest);
                break;

            case TLSX_STATUS_REQUEST_V2:
                WOLFSSL_MSG("Certificate Status Request v2 extension received");
            #ifdef WOLFSSL_DEBUG_TLS
                WOLFSSL_BUFFER(input + offset, size);
            #endif

#if defined(WOLFSSL_TLS13) && defined(HAVE_CERTIFICATE_STATUS_REQUEST_V2)
                if (IsAtLeastTLSv1_3(ssl->version) &&
                        msgType != client_hello &&
                        msgType != certificate_request &&
                        msgType != certificate) {
                    return EXT_NOT_ALLOWED;
                }
#endif
                ret = CSR2_PARSE(ssl, input + offset, size, isRequest);
                break;

#ifdef HAVE_EXTENDED_MASTER
            case HELLO_EXT_EXTMS:
                WOLFSSL_MSG("Extended Master Secret extension received");
            #ifdef WOLFSSL_DEBUG_TLS
                WOLFSSL_BUFFER(input + offset, size);
            #endif

#if defined(WOLFSSL_TLS13)
                if (IsAtLeastTLSv1_3(ssl->version))
                    break;
#endif
                if (size != 0)
                    return BUFFER_ERROR;

#ifndef NO_WOLFSSL_SERVER
                if (isRequest)
                    ssl->options.haveEMS = 1;
#endif
                pendingEMS = 1;
                break;
#endif

            case TLSX_RENEGOTIATION_INFO:
                WOLFSSL_MSG("Secure Renegotiation extension received");
            #ifdef WOLFSSL_DEBUG_TLS
                WOLFSSL_BUFFER(input + offset, size);
            #endif

#if defined(WOLFSSL_TLS13) && defined(HAVE_SERVER_RENEGOTIATION_INFO)
                if (IsAtLeastTLSv1_3(ssl->version))
                    break;
#endif
                ret = SCR_PARSE(ssl, input + offset, size, isRequest);
                break;

            case TLSX_SESSION_TICKET:
                WOLFSSL_MSG("Session Ticket extension received");
            #ifdef WOLFSSL_DEBUG_TLS
                WOLFSSL_BUFFER(input + offset, size);
            #endif

#if defined(WOLFSSL_TLS13) && defined(HAVE_SESSION_TICKET)
                if (IsAtLeastTLSv1_3(ssl->version) &&
                        msgType != client_hello) {
                    return EXT_NOT_ALLOWED;
                }
#endif
                ret = WOLF_STK_PARSE(ssl, input + offset, size, isRequest);
                break;

            case TLSX_APPLICATION_LAYER_PROTOCOL:
                WOLFSSL_MSG("ALPN extension received");

            #ifdef WOLFSSL_DEBUG_TLS
                WOLFSSL_BUFFER(input + offset, size);
            #endif

#if defined(WOLFSSL_TLS13) && defined(HAVE_ALPN)
                if (IsAtLeastTLSv1_3(ssl->version) &&
                        msgType != client_hello &&
                        msgType != encrypted_extensions) {
                    return EXT_NOT_ALLOWED;
                }
                else if (!IsAtLeastTLSv1_3(ssl->version) &&
                         msgType == encrypted_extensions) {
                    return EXT_NOT_ALLOWED;
                }
#endif
                ret = ALPN_PARSE(ssl, input + offset, size, isRequest);
                break;
#if !defined(NO_CERTS) && !defined(WOLFSSL_NO_SIGALG)
            case TLSX_SIGNATURE_ALGORITHMS:
                WOLFSSL_MSG("Signature Algorithms extension received");
            #ifdef WOLFSSL_DEBUG_TLS
                WOLFSSL_BUFFER(input + offset, size);
            #endif

                if (!IsAtLeastTLSv1_2(ssl))
                    break;
            #ifdef WOLFSSL_TLS13
                if (IsAtLeastTLSv1_3(ssl->version) &&
                        msgType != client_hello &&
                        msgType != certificate_request) {
                    return EXT_NOT_ALLOWED;
                }
            #endif
                ret = SA_PARSE(ssl, input + offset, size, isRequest, suites);
                break;
#endif

#if defined(HAVE_ENCRYPT_THEN_MAC) && !defined(WOLFSSL_AEAD_ONLY)
            case TLSX_ENCRYPT_THEN_MAC:
                WOLFSSL_MSG("Encrypt-Then-Mac extension received");

                /* Ignore for TLS 1.3+ */
                if (IsAtLeastTLSv1_3(ssl->version))
                    break;

                ret = ETM_PARSE(ssl, input + offset, size, msgType);
                break;
#endif /* HAVE_ENCRYPT_THEN_MAC */

#ifdef WOLFSSL_TLS13
            case TLSX_SUPPORTED_VERSIONS:
                WOLFSSL_MSG("Skipping Supported Versions - already processed");
            #ifdef WOLFSSL_DEBUG_TLS
                WOLFSSL_BUFFER(input + offset, size);
            #endif

                break;

    #ifdef WOLFSSL_SEND_HRR_COOKIE
            case TLSX_COOKIE:
                WOLFSSL_MSG("Cookie extension received");
            #ifdef WOLFSSL_DEBUG_TLS
                WOLFSSL_BUFFER(input + offset, size);
            #endif

                if (!IsAtLeastTLSv1_3(ssl->version))
                    break;

                if (msgType != client_hello &&
                        msgType != hello_retry_request) {
                    return EXT_NOT_ALLOWED;
                }

                ret = CKE_PARSE(ssl, input + offset, size, msgType);
                break;
    #endif

    #if defined(HAVE_SESSION_TICKET) || !defined(NO_PSK)
            case TLSX_PRE_SHARED_KEY:
                WOLFSSL_MSG("Pre-Shared Key extension received");
            #ifdef WOLFSSL_DEBUG_TLS
                WOLFSSL_BUFFER(input + offset, size);
            #endif

                if (!IsAtLeastTLSv1_3(ssl->version))
                    break;

                if (msgType != client_hello && msgType != server_hello) {
                    WOLFSSL_ERROR_VERBOSE(EXT_NOT_ALLOWED);
                    return EXT_NOT_ALLOWED;
                }

                ret = PSK_PARSE(ssl, input + offset, size, msgType);
                pskDone = 1;
                break;

            case TLSX_PSK_KEY_EXCHANGE_MODES:
                WOLFSSL_MSG("PSK Key Exchange Modes extension received");
            #ifdef WOLFSSL_DEBUG_TLS
                WOLFSSL_BUFFER(input + offset, size);
            #endif

                if (!IsAtLeastTLSv1_3(ssl->version))
                    break;

                if (msgType != client_hello) {
                    WOLFSSL_ERROR_VERBOSE(EXT_NOT_ALLOWED);
                    return EXT_NOT_ALLOWED;
                }

                ret = PKM_PARSE(ssl, input + offset, size, msgType);
                break;
    #endif

    #ifdef WOLFSSL_EARLY_DATA
            case TLSX_EARLY_DATA:
                WOLFSSL_MSG("Early Data extension received");
            #ifdef WOLFSSL_DEBUG_TLS
                WOLFSSL_BUFFER(input + offset, size);
            #endif

                if (!IsAtLeastTLSv1_3(ssl->version))
                    break;

                if (msgType != client_hello && msgType != session_ticket &&
                        msgType != encrypted_extensions) {
                    WOLFSSL_ERROR_VERBOSE(EXT_NOT_ALLOWED);
                    return EXT_NOT_ALLOWED;
                }
                if (!IsAtLeastTLSv1_3(ssl->version) &&
                        (msgType == session_ticket ||
                         msgType == encrypted_extensions)) {
                    WOLFSSL_ERROR_VERBOSE(EXT_NOT_ALLOWED);
                    return EXT_NOT_ALLOWED;
                }
                ret = EDI_PARSE(ssl, input + offset, size, msgType);
                break;
    #endif

    #ifdef WOLFSSL_POST_HANDSHAKE_AUTH
            case TLSX_POST_HANDSHAKE_AUTH:
                WOLFSSL_MSG("Post Handshake Authentication extension received");
            #ifdef WOLFSSL_DEBUG_TLS
                WOLFSSL_BUFFER(input + offset, size);
            #endif

                if (!IsAtLeastTLSv1_3(ssl->version))
                    break;

                if (msgType != client_hello) {
                    WOLFSSL_ERROR_VERBOSE(EXT_NOT_ALLOWED);
                    return EXT_NOT_ALLOWED;
                }

                ret = PHA_PARSE(ssl, input + offset, size, msgType);
                break;
    #endif

    #if !defined(NO_CERTS) && !defined(WOLFSSL_NO_SIGALG)
            case TLSX_SIGNATURE_ALGORITHMS_CERT:
                WOLFSSL_MSG("Signature Algorithms extension received");
            #ifdef WOLFSSL_DEBUG_TLS
                WOLFSSL_BUFFER(input + offset, size);
            #endif

                if (!IsAtLeastTLSv1_3(ssl->version))
                    break;

                if (msgType != client_hello &&
                        msgType != certificate_request) {
                    WOLFSSL_ERROR_VERBOSE(EXT_NOT_ALLOWED);
                    return EXT_NOT_ALLOWED;
                }
                if (!IsAtLeastTLSv1_3(ssl->version) &&
                        msgType == certificate_request) {
                    WOLFSSL_ERROR_VERBOSE(EXT_NOT_ALLOWED);
                    return EXT_NOT_ALLOWED;
                }

                ret = SAC_PARSE(ssl, input + offset, size, isRequest);
                break;
    #endif

            case TLSX_KEY_SHARE:
                WOLFSSL_MSG("Key Share extension received");
            #ifdef WOLFSSL_DEBUG_TLS
                WOLFSSL_BUFFER(input + offset, size);
            #endif

    #ifdef HAVE_SUPPORTED_CURVES
                if (!IsAtLeastTLSv1_3(ssl->version))
                    break;

                if (msgType != client_hello && msgType != server_hello &&
                        msgType != hello_retry_request) {
                    WOLFSSL_ERROR_VERBOSE(EXT_NOT_ALLOWED);
                    return EXT_NOT_ALLOWED;
                }
    #endif

                ret = KS_PARSE(ssl, input + offset, size, msgType);
                break;
#endif