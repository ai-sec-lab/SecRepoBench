static enum ofperr
decode_NXAST_RAW_ENCAP(const struct nx_action_encap *nae,
                       enum ofp_version ofp_version OVS_UNUSED,
                       struct ofpbuf *buffer)
{
    struct ofpact_encap *encap;
    const struct ofp_ed_prop_header *ofp_prop;
    // <MASK>
    encap->n_props = n_props;
    buffer->header = &encap->ofpact;
    ofpact_finish_ENCAP(buffer, &encap);

    return 0;
}