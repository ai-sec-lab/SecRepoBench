static int
parse_odp_push_nsh_action(const char *s, struct ofpbuf *actions)
{
    int n = 0;
    int ret = 0;
    uint32_t spi = 0;
    uint8_t si = 255;
    uint32_t cd;
    struct ovs_key_nsh nsh;
    uint8_t metadata[NSH_CTX_HDRS_MAX_LEN];
    uint8_t md_size = 0;

    if (!ovs_scan_len(s, &n, "push_nsh(")) {
        ret = -EINVAL;
        goto out;
    }

    /* The default is NSH_M_TYPE1 */
    nsh.flags = 0;
    nsh.ttl = 63;
    nsh.mdtype = NSH_M_TYPE1;
    nsh.np = NSH_P_ETHERNET;
    nsh.path_hdr = nsh_spi_si_to_path_hdr(0, 255);
    memset(nsh.context, 0, NSH_M_TYPE1_MDLEN);

    for (;;) {
        n += strspn(s + n, delimiters);
        if (s[n] == ')') {
            break;
        }

        if (ovs_scan_len(s, &n, "flags=%"SCNi8, &nsh.flags)) {
            continue;
        }
        if (ovs_scan_len(s, &n, "ttl=%"SCNi8, &nsh.ttl)) {
            continue;
        }
        if (ovs_scan_len(s, &n, "mdtype=%"SCNi8, &nsh.mdtype)) {
            switch (nsh.mdtype) {
            case NSH_M_TYPE1:
                /* This is the default format. */;
                break;
            case NSH_M_TYPE2:
                /* Length will be updated later. */
                md_size = 0;
                break;
            default:
                ret = -EINVAL;
                goto out;
            }
            continue;
        }
        if (ovs_scan_len(s, &n, "np=%"SCNi8, &nsh.np)) {
            continue;
        }
        if (ovs_scan_len(s, &n, "spi=0x%"SCNx32, &spi)) {
            continue;
        }
        if (ovs_scan_len(s, &n, "si=%"SCNi8, &si)) {
            continue;
        }
        if (nsh.mdtype == NSH_M_TYPE1) {
            if (ovs_scan_len(s, &n, "c1=0x%"SCNx32, &cd)) {
                nsh.context[0] = htonl(cd);
                continue;
            }
            if (ovs_scan_len(s, &n, "c2=0x%"SCNx32, &cd)) {
                nsh.context[1] = htonl(cd);
                continue;
            }
            if (ovs_scan_len(s, &n, "c3=0x%"SCNx32, &cd)) {
                nsh.context[2] = htonl(cd);
                continue;
            }
            if (ovs_scan_len(s, &n, "c4=0x%"SCNx32, &cd)) {
                nsh.context[3] = htonl(cd);
                continue;
            }
        }
        else if (nsh.mdtype == NSH_M_TYPE2) {
            struct ofpbuf b;
            char buf[512];
            size_t mdlen, padding;
            if (ovs_scan_len(s, &n, "md2=0x%511[0-9a-fA-F]", buf)
                && n/2 <= sizeof metadata) {
                ofpbuf_use_stub(&b, metadata, sizeof metadata);
                ofpbuf_put_hex(&b, buf, &mdlen);
                /* Pad metadata to 4 bytes. */
                padding = PAD_SIZE(mdlen, 4);
                if (padding > 0) {
                    ofpbuf_push_zeros(&b, padding);
                }
                md_size = mdlen + padding;
                ofpbuf_uninit(&b);
                continue;
            }
        }

        ret = -EINVAL;
        goto out;
    }
out:
    if (ret >= 0) {
        nsh.path_hdr = nsh_spi_si_to_path_hdr(spi, si);
        size_t offset = nl_msg_start_nested(actions, OVS_ACTION_ATTR_PUSH_NSH);
        nsh_key_to_attr(actions, &nsh, metadata, md_size, false);
        nl_msg_end_nested(actions, offset);
        ret = n;
    }
    return ret;
}