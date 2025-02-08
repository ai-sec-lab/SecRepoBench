static enum ofperr
decode_NXAST_RAW_ENCAP(const struct nx_action_encap *nae,
                       enum ofp_version ofp_version OVS_UNUSED,
                       struct ofpbuf *out)
{
    struct ofpact_encap *encap;
    const struct ofp_ed_prop_header *ofp_prop;
    // <MASK>
    encap->n_props = n_props;
    out->header = &encap->ofpact;
    ofpact_finish_ENCAP(out, &encap);

    return 0;
}