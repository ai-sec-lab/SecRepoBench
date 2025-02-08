otError Address::FromString(const char *aBuf)
{
    otError     error  = OT_ERROR_NONE;
    uint8_t *   dst    = reinterpret_cast<uint8_t *>(mFields.m8);
    uint8_t *   endp   = reinterpret_cast<uint8_t *>(mFields.m8 + 15);
    uint8_t *   colonp = NULL;
    const char *colonc = NULL;
    uint16_t    val    = 0;
    uint8_t     count  = 0;
    bool        first  = true;
    bool        hasIp4 = false;
    char        ch;
    uint8_t     d;

    memset(mFields.m8, 0, 16);

    dst--;

    for (;;)
    {
        ch = *aBuf++;
        d  = ch & 0xf;

        if (('a' <= ch && ch <= 'f') || ('A' <= ch && ch <= 'F'))
        {
            d += 9;
        }
        else if (ch == ':' || ch == '\0' || ch == ' ')
        {
            if (count)
            {
                VerifyOrExit(dst + 2 <= endp, error = OT_ERROR_PARSE);
                *(dst + 1) = static_cast<uint8_t>(val >> 8);
                *(dst + 2) = static_cast<uint8_t>(val);
                dst += 2;
                count = 0;
                val   = 0;
            }
            else if (ch == ':')
            {
                VerifyOrExit(colonp == NULL || first, error = OT_ERROR_PARSE);
                colonp = dst;
            }

            if (ch == '\0' || ch == ' ')
            {
                break;
            }

            colonc = aBuf;

            continue;
        }
        else if (ch == '.')
        {
            hasIp4 = true;

            // Do not count bytes of the embedded IPv4 address.
            endp -= kIp4AddressSize;

            VerifyOrExit(dst <= endp, error = OT_ERROR_PARSE);

            break;
        }
        else
        {
            VerifyOrExit('0' <= ch && ch <= '9', error = OT_ERROR_PARSE);
        }

        first = false;
        val   = static_cast<uint16_t>((val << 4) | d);
        VerifyOrExit(++count <= 4, error = OT_ERROR_PARSE);
    }

    VerifyOrExit(colonp || dst == endp, error = OT_ERROR_PARSE);

    while (colonp && dst > colonp)
    {
        *endp-- = *dst--;
    }

    while (endp > dst)
    {
        *endp-- = 0;
    }

    if (hasIp4)
    {
        val = 0;

        // Reset the start and end pointers.
        dst  = reinterpret_cast<uint8_t *>(mFields.m8 + 12);
        endp = reinterpret_cast<uint8_t *>(mFields.m8 + 15);

        for (;;)
        {
            ch = *colonc++;

            if (ch == '.' || ch == '\0' || ch == ' ')
            {
                VerifyOrExit(dst <= endp, error = OT_ERROR_PARSE);

                *dst++ = static_cast<uint8_t>(val);
                val    = 0;

                if (ch == '\0' || ch == ' ')
                {
                    // Check if embedded IPv4 address had exactly four parts.
                    VerifyOrExit(dst == endp + 1, error = OT_ERROR_PARSE);
                    break;
                }
            }
            else
            {
                VerifyOrExit('0' <= ch && ch <= '9', error = OT_ERROR_PARSE);

                val = (10 * val) + (ch & 0xf);

                // Single part of IPv4 address has to fit in one byte.
                VerifyOrExit(val <= 0xff, error = OT_ERROR_PARSE);
            }
        }
    }

exit:
    return error;
}