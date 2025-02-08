static int dissect_rf4ce_nwk_common(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *data _U_)
{
    guint currentoffset = 0;
    gboolean success;
    guint8 *decrypted = (guint8 *)wmem_alloc(pinfo->pool, 512);
    guint8 src_addr[RF4CE_IEEE_ADDR_LEN] = {0};
    guint8 dst_addr[RF4CE_IEEE_ADDR_LEN] = {0};

    guint8 fcf = 0xff;
    guint8 frame_type = 0xff;
    guint8 profile_id = 0xff;
    guint16 size;

    proto_item *ti = proto_tree_add_item(tree, proto_rf4ce_nwk, tvb, 0, -1, ENC_LITTLE_ENDIAN);
    proto_tree *rf4ce_nwk_tree = proto_item_add_subtree(ti, ett_rf4ce_nwk);

    static int *const nwk_fcf_bits[] = {
        &hf_rf4ce_nwk_fcf_frame_type,
        &hf_rf4ce_nwk_fcf_security_enabled,
        &hf_rf4ce_nwk_fcf_protocol_version,
        &hf_rf4ce_nwk_fcf_reserved,
        &hf_rf4ce_nwk_fcf_channel_designator,
        NULL};

    proto_tree_add_bitmask(rf4ce_nwk_tree, tvb, currentoffset, hf_rf4ce_nwk_fcf, ett_rf4ce_nwk, nwk_fcf_bits, ENC_LITTLE_ENDIAN);
    fcf = tvb_get_guint8(tvb, currentoffset);
    currentoffset += 1;

    proto_tree_add_item(rf4ce_nwk_tree, hf_rf4ce_nwk_seq_num, tvb, currentoffset, 4, ENC_LITTLE_ENDIAN);
    currentoffset += 4;

    frame_type = fcf & RF4CE_NWK_FCF_FRAME_TYPE_MASK;

    if (frame_type == RF4CE_NWK_FCF_FRAME_TYPE_DATA || frame_type == RF4CE_NWK_FCF_FRAME_TYPE_VENDOR_SPECIFIC)
    {
        proto_tree_add_item(rf4ce_nwk_tree, hf_rf4ce_nwk_profile_id, tvb, currentoffset, 1, ENC_LITTLE_ENDIAN);
        profile_id = tvb_get_guint8(tvb, currentoffset);
        currentoffset += 1;
    }

    if (frame_type == RF4CE_NWK_FCF_FRAME_TYPE_VENDOR_SPECIFIC)
    {
        proto_tree_add_item(rf4ce_nwk_tree, hf_rf4ce_nwk_vendor_id, tvb, currentoffset, 2, ENC_LITTLE_ENDIAN);
        currentoffset += 2;
    }

    rf4ce_addr_table_get_ieee_addr(src_addr, pinfo, TRUE);
    rf4ce_addr_table_get_ieee_addr(dst_addr, pinfo, FALSE);

    // <MASK>
}