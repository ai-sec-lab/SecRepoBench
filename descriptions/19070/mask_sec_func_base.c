static int dissect_1722_acf_lin(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void* data _U_)
{
    proto_item *ti;
    proto_item *ti_lin;
    proto_tree *tree_lin;
    proto_tree *tree_flags;
    guint       offset = 0;
    guint       captured_length = tvb_captured_length(tvb);
    guint32     pad_length;
    gboolean    mtv;
    guint32     bus_id;
    guint32     lin_id;
    gint        payload_length;

    ti_lin = proto_tree_add_item(tree, proto_1722_acf_lin, tvb, offset, -1, ENC_NA);
    tree_lin = proto_item_add_subtree(ti_lin, ett_1722_lin);

    if (captured_length < IEEE_1722_ACF_LIN_HEADER_SIZE) {
        expert_add_info(pinfo, ti_lin, &ei_1722_lin_header_cropped);
        return captured_length;
    }

    tree_flags = proto_tree_add_subtree(tree_lin, tvb, offset, 1, ett_1722_lin_flags, &ti, "Flags and BusID");
    proto_tree_add_item_ret_uint(tree_flags, hf_1722_lin_pad, tvb, offset, 1, ENC_BIG_ENDIAN, &pad_length);
    proto_tree_add_item_ret_boolean(tree_flags, hf_1722_lin_mtv, tvb, offset, 1, ENC_BIG_ENDIAN, &mtv);
    proto_tree_add_item_ret_uint(tree_flags, hf_1722_lin_bus_id, tvb, offset, 1, ENC_BIG_ENDIAN, &bus_id);
    proto_item_append_text(ti, ": pad=%u, mtv=%u, bus_id=%u", pad_length, (unsigned)mtv, bus_id);
    offset += 1;

    proto_tree_add_item_ret_uint(tree_lin, hf_1722_lin_identifier, tvb, offset, 1, ENC_BIG_ENDIAN, &lin_id);
    offset += 1;

    proto_tree_add_item(tree_lin, hf_1722_lin_message_timestamp, tvb, offset, 8, ENC_BIG_ENDIAN);
    offset += 8;

    describe_lin_message(ti_lin, bus_id, lin_id);
    describe_lin_message(proto_tree_get_parent(tree), bus_id, lin_id);
    col_set_str(pinfo->cinfo, COL_PROTOCOL, "ACF-LIN");
    col_clear(pinfo->cinfo, COL_INFO);
    col_add_fstr(pinfo->cinfo, COL_INFO, "ACF-LIN(%u): 0x%02x   ", bus_id, lin_id);

    // <MASK>
}