// Do not count bytes of the embedded IPv4 address.
            endp -= kIp4AddressSize;

            VerifyOrExit(dst <= endp, error = OT_ERROR_PARSE);

            break;