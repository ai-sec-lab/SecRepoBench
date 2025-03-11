PLIST_API int plist_from_openstep(const char *plistopenstep, uint32_t length, plist_t * plist)
{
    if (!plist) {
        return PLIST_ERR_INVALID_ARG;
    }
    *plist = NULL;
    if (!plistopenstep || (length == 0)) {
        return PLIST_ERR_INVALID_ARG;
    }

    struct _parse_ctx ctx = { plistopenstep, plistopenstep, plistopenstep + length, 0 };

    int err = node_from_openstep(&ctx, plist);
    if (err == 0) {
        if (!*plist) {
            /* whitespace only file is considered an empty dictionary */
            *plist = plist_new_dict();
        } else if (ctx.pos < ctx.end && *ctx.pos == '=') // <MASK>
    }

    return err;
}