size_t props_len;
    uint16_t n_props = 0;
    int err;

    encap = ofpact_put_ENCAP(out);
    encap->ofpact.raw = NXAST_RAW_ENCAP;
    switch (ntohl(nae->new_pkt_type)) {
    case PT_ETH:
    case PT_NSH:
        /* Add supported encap header types here. */
        break;
    default:
        return OFPERR_NXBAC_BAD_HEADER_TYPE;
    }
    encap->new_pkt_type = nae->new_pkt_type;
    encap->hdr_size = ntohs(nae->hdr_size);

    ofp_prop = nae->props;
    props_len = ntohs(nae->len) - offsetof(struct nx_action_encap, props);
    n_props = 0;
    while (props_len > 0) {
        err = decode_ed_prop(&ofp_prop, out, &props_len);
        if (err) {
            return err;
        }
        n_props++;
    }