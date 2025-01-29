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