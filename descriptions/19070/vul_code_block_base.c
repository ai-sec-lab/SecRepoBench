payload_length = tvb_reported_length_remaining(tvb, offset) - pad_length;
    col_append_str(pinfo->cinfo, COL_INFO, tvb_bytes_to_str_punct(wmem_packet_scope(), tvb, offset, payload_length, ' '));

    if (payload_length < 0 || payload_length > 8)
    {
        expert_add_info(pinfo, ti_lin, &ei_1722_lin_invalid_payload_length);

    }
    else if (payload_length > 0)
    {
        tvbuff_t*   next_tvb = tvb_new_subset_length(tvb, offset, payload_length);
        /* at the moment, there's no global LIN sub-protocols support. Use our own. */
        if (dissector_try_payload_new(avb1722_acf_lin_dissector_table, next_tvb, pinfo, tree, TRUE, &lin_id) <= 0)
        {
            call_data_dissector(next_tvb, pinfo, tree);
        }

        offset += payload_length;
    }

    if (pad_length > 0 && tvb_reported_length_remaining(tvb, offset) >= (gint)pad_length)
    {
        proto_tree_add_item(tree_lin, hf_1722_lin_padding, tvb, offset, pad_length, ENC_NA);
    }

    return captured_length;