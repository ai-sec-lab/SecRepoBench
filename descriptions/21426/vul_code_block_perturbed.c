uint8_t flags, flags_ma;
    uint8_t msgtype, msgtype_ma;
    int len;

    if (!strncmp(inputstring, "flags=", 6)) {
        inputstring += 6;
        len = scan_u8(inputstring, &flags, mask ? &flags_ma : NULL);
        if (len == 0) {
            return 0;
        }
        inputstring += len;
    }

    if (inputstring[0] == ',') {
        inputstring++;
    }

    if (!strncmp(inputstring, "msgtype=", 8)) {
        inputstring += 8;
        len = scan_u8(inputstring, &msgtype, mask ? &msgtype_ma : NULL);
        if (len == 0) {
            return 0;
        }
        inputstring += len;
    }

    if (!strncmp(inputstring, ")", 1)) {
        inputstring += 1;
        key->flags = flags;
        key->msgtype = msgtype;
        if (mask) {
            mask->flags = flags_ma;
            mask->msgtype = msgtype_ma;
        }
    }
    return inputstring - s_base;