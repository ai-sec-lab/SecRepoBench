static int
scan_gtpu_metadata(const char *s,
                   struct gtpu_metadata *key,
                   struct gtpu_metadata *mask)
{
    const char *s_base = s;
    uint8_t flags = 0, flags_ma = 0;
    uint8_t msgtype = 0, msgtype_ma = 0;
    int len;

    if (!strncmp(s, "flags=", 6)) {
        s += 6;
        len = scan_u8(s, &flags, mask ? &flags_ma : NULL);
        if (len == 0) {
            return 0;
        }
        s += len;
    }

    if (s[0] == ',') {
        s++;
    }

    if (!strncmp(s, "msgtype=", 8)) {
        s += 8;
        len = scan_u8(s, &msgtype, mask ? &msgtype_ma : NULL);
        if (len == 0) {
            return 0;
        }
        s += len;
    }

    if (!strncmp(s, ")", 1)) {
        s += 1;
        key->flags = flags;
        key->msgtype = msgtype;
        if (mask) {
            mask->flags = flags_ma;
            mask->msgtype = msgtype_ma;
        }
    }
    return s - s_base;
}