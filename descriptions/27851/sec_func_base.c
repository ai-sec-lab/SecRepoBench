static enum ofperr
decode_NXAST_RAW_ENCAP(const struct nx_action_encap *nae,
                       enum ofp_version ofp_version OVS_UNUSED,
                       struct ofpbuf *out)
{
    struct ofpact_encap *encap;
    const struct ofp_ed_prop_header *ofp_prop;
    const size_t encap_ofs = out->size;
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
    encap = ofpbuf_at_assert(out, encap_ofs, sizeof *encap);
    encap->n_props = n_props;
    out->header = &encap->ofpact;
    ofpact_finish_ENCAP(out, &encap);

    return 0;
}