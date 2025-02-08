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

    size = tvb_captured_length_remaining(tvb, 0);

    if (fcf & RF4CE_NWK_FCF_SECURITY_MASK)
    {
        success = decrypt_data(
            tvb_get_ptr(tvb, 0, size),
            decrypted,
            currentoffset,
            &size,
            src_addr,
            dst_addr);
    }
    else if (size > currentoffset)
    {
        size -= currentoffset;
        tvb_memcpy(tvb, decrypted, currentoffset, size);
        success = TRUE;
    }
    else
    {
        success = FALSE;
    }

    if (success)
    {
        guint decrypted_offset = 0;

        /* On decryption success: replace the tvb, make offset point to its beginning */
        tvb = tvb_new_child_real_data(tvb, decrypted, size, size);
        add_new_data_source(pinfo, tvb, "CCM* decrypted payload");

        if (frame_type == RF4CE_NWK_FCF_FRAME_TYPE_CMD)
        {
            proto_tree *nwk_payload_tree = proto_tree_add_subtree(rf4ce_nwk_tree, tvb, decrypted_offset, tvb_captured_length(tvb) - decrypted_offset, ett_rf4ce_nwk_payload, NULL, "NWK Payload");
            dissect_rf4ce_nwk_cmd(tvb, pinfo, nwk_payload_tree, &decrypted_offset);
        }
        else if (frame_type == RF4CE_NWK_FCF_FRAME_TYPE_DATA)
        {
            if (profile_id == RF4CE_NWK_PROFILE_ID_GDP)
            {
                decrypted_offset += call_dissector_with_data(rf4ce_gdp_handle, tvb, pinfo, tree, (void *)("GDP"));
            }
            else if (profile_id == RF4CE_NWK_PROFILE_ID_ZRC20)
            {
                decrypted_offset += call_dissector_with_data(rf4ce_gdp_handle, tvb, pinfo, tree, (void *)("ZRC 2.0"));
            }
            else if (profile_id == RF4CE_NWK_PROFILE_ID_ZRC10)
            {
                decrypted_offset += call_dissector_with_data(rf4ce_gdp_handle, tvb, pinfo, tree, (void *)("ZRC 1.0"));
            }
        }

        currentoffset += decrypted_offset;
    }
    else
    {
        /* On decryption error: make offset point to the end of original tvb */
        currentoffset = tvb_reported_length(tvb);
    }

    if (currentoffset < tvb_captured_length(tvb))
    {
        guint unparsed_length = tvb_captured_length(tvb) - currentoffset;
        proto_tree_add_item(rf4ce_nwk_tree, hf_rf4ce_nwk_unparsed_payload, tvb, currentoffset, unparsed_length, ENC_NA);
#if 0
        /* enable this block if you need to add NWK MIC */
        currentoffset += unparsed_length;
#endif
    }

#if 0
  if (fcf & RF4CE_NWK_FCF_SECURITY_MASK)
  {
    proto_tree_add_item(rf4ce_nwk_tree, hf_rf4ce_nwk_mic, tvb, currentoffset, 4, ENC_LITTLE_ENDIAN);
    currentoffset += 4;
  }
#endif

    return tvb_captured_length(tvb);
}