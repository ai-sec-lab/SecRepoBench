static int
parse_odp_push_nsh_action(const char *str, struct ofpbuf *actions)
{
    int n = 0;
    int ret = 0;
    uint32_t spi = 0;
    uint8_t si = 255;
    uint32_t cd;
    struct ovs_key_nsh nsh;
    uint8_t metadata[NSH_CTX_HDRS_MAX_LEN];
    uint8_t md_size = 0;

    if (!ovs_scan_len(str, &n, "push_nsh(")) {
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
        n += strspn(str + n, delimiters);
        if (str[n] == ')') {
            break;
        }

        if (ovs_scan_len(str, &n, "flags=%"SCNi8, &nsh.flags)) {
            continue;
        }
        if (ovs_scan_len(str, &n, "ttl=%"SCNi8, &nsh.ttl)) {
            continue;
        }
        if (ovs_scan_len(str, &n, "mdtype=%"SCNi8, &nsh.mdtype)) {
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
        if (ovs_scan_len(str, &n, "np=%"SCNi8, &nsh.np)) {
            continue;
        }
        if (ovs_scan_len(str, &n, "spi=0x%"SCNx32, &spi)) {
            continue;
        }
        if (ovs_scan_len(str, &n, "si=%"SCNi8, &si)) {
            continue;
        }
        if (nsh.mdtype == NSH_M_TYPE1) {
            if (ovs_scan_len(str, &n, "c1=0x%"SCNx32, &cd)) {
                nsh.context[0] = htonl(cd);
                continue;
            }
            if (ovs_scan_len(str, &n, "c2=0x%"SCNx32, &cd)) {
                nsh.context[1] = htonl(cd);
                continue;
            }
            if (ovs_scan_len(str, &n, "c3=0x%"SCNx32, &cd)) {
                nsh.context[2] = htonl(cd);
                continue;
            }
            if (ovs_scan_len(str, &n, "c4=0x%"SCNx32, &cd)) {
                nsh.context[3] = htonl(cd);
                continue;
            }
        }
        else if (nsh.mdtype == NSH_M_TYPE2) {
            // <MASK>
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