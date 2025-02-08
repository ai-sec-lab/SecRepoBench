static int
dissect_alp_mpegts(tvbuff_t *tvb, gint offset, packet_info *pinfo, proto_tree *tree, proto_tree *alp_tree)
{
    guint8 headerbyte = tvb_get_guint8(tvb, offset);
    guint8 ahf = headerbyte & ALP_MPEGTS_AHF_MASK;
    guint8 numts = (headerbyte & ALP_MPEGTS_NUMTS_MASK) >> 1;
    if (numts == 0) {
        numts = 16;
    }

    PROTO_ITEM_SET_GENERATED(
        proto_tree_add_uint(alp_tree, hf_alp_mpegts_numts, tvb, offset, 1, numts)
    );
    proto_tree_add_item(alp_tree, hf_alp_mpegts_ahf, tvb, offset, 1, ENC_BIG_ENDIAN);
    offset++;

    guint8 hdm = 0;
    guint8 dnp = 0;

    if (ahf) {
        guint8 header1 = tvb_get_guint8(tvb, offset);
        hdm = header1 & ALP_MPEGTS_HDM_MASK;
        dnp = header1 & ALP_MPEGTS_DNP_MASK;
        if ((hdm == 0) && (dnp == 0)) {
            dnp = 128;
        }

        proto_tree_add_item(alp_tree, hf_alp_mpegts_hdm, tvb, offset, 1, ENC_BIG_ENDIAN);
        PROTO_ITEM_SET_GENERATED(
            proto_tree_add_uint(alp_tree, hf_alp_mpegts_dnp, tvb, offset, 1, dnp)
        );
        offset++;
    }

    while (dnp--) {
        guchar *ts_frame = (guchar*)wmem_alloc(pinfo->pool, 188);

        ts_frame[0] = 0x47;
        ts_frame[1] = 0x1F;
        ts_frame[2] = 0xFF;
        ts_frame[3] = 0x10;
        ts_frame[4] = 0x00;
        memset(ts_frame + 5, 0xFF, 183);

        tvbuff_t *ts_frame_tvb = tvb_new_child_real_data(tvb, ts_frame, 188, 188);
        call_dissector(ts_handle, ts_frame_tvb, pinfo, tree);
    }

    guchar *ts_frame = (guchar*)wmem_alloc(pinfo->pool, 188);

    ts_frame[0] = 0x47;
    // <MASK>

    if (offset < (gint)tvb_captured_length(tvb)) {
        gint junk_length = tvb_captured_length(tvb) - offset;
        proto_tree_add_bytes_format(alp_tree, hf_alp_junk, tvb, offset, -1, NULL, "Junk at end (%u byte%s)", junk_length, (junk_length == 1) ? "" : "s");
    }

    return tvb_captured_length(tvb);
}