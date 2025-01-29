if (parseCtx->index >= parseCtx->tokenCount)
                        /* We got invalid json. See https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=14620 */
                        return UA_STATUSCODE_BADOUTOFRANGE;
                    *resultIndex = parseCtx->index;
                    return UA_STATUSCODE_GOOD;