if (len == 0) {
        proto_tree_add_expert_format(
            parent_tree, actx->pinfo, &ei_ber_constr_bitstr, tvb, offset, len,
            "dissect_ber_constrained_bitstring(): frame:%u offset:%d Was passed an illegal length of 0",
            actx->pinfo->num, offset);
        return offset;
    }
    actx->created_item = NULL;

    if (pc) {
        /* constructed */
        /* TO DO */
    } else {
        /* primitive */
        pad = tvb_get_guint8(tvb, offset);
        if ((pad == 0) && (len == 1)) {
            /* empty */
            proto_tree_add_item(parent_tree, hf_ber_bitstring_empty, tvb, offset, 1, ENC_BIG_ENDIAN);
        } else {
            /* padding */
            proto_item *pad_item = proto_tree_add_item(parent_tree, hf_ber_bitstring_padding, tvb, offset, 1, ENC_BIG_ENDIAN);
            if (pad > 7) {
                expert_add_info_format(
                    actx->pinfo, pad_item, &ei_ber_illegal_padding,
                    "Illegal padding (0 .. 7): %d", pad);
            }
        }
        offset++;
        len--;
        if (hf_id >= 0) {
            item = proto_tree_add_item(parent_tree, hf_id, tvb, offset, len, ENC_BIG_ENDIAN);
            actx->created_item = item;
            if (ett_id != -1) {
                tree = proto_item_add_subtree(item, ett_id);
            }
        }
        if (out_tvb) {
            *out_tvb = ber_tvb_new_subset_length(tvb, offset, len);
        }
    }

    if (named_bits) {
        sep  = " (";
        term = FALSE;
        nb = named_bits;
        bitstring = (guint8 *)tvb_memdup(wmem_packet_scope(), tvb, offset, len);

        while (nb->p_id) {
            if ((len > 0) && (pad < 8*len) && (nb->bit < (8*len-pad))) {
                val = tvb_get_guint8(tvb, offset + nb->bit/8);
                bitstring[(nb->bit/8)] &= ~(0x80 >> (nb->bit%8));
                val &= 0x80 >> (nb->bit%8);
                b0 = (nb->gb0 == -1) ? nb->bit/8 :
                               ((guint32)nb->gb0)/8;
                b1 = (nb->gb1 == -1) ? nb->bit/8 :
                               ((guint32)nb->gb1)/8;
                proto_tree_add_item(tree, *(nb->p_id), tvb, offset + b0, b1 - b0 + 1, ENC_BIG_ENDIAN);
            } else {  /* 8.6.2.4 */
                val = 0;
                proto_tree_add_boolean(tree, *(nb->p_id), tvb, offset + len, 0, 0x00);
            }
            if (val) {
                if (item && nb->tstr) {
                    proto_item_append_text(item, "%s%s", sep, nb->tstr);
                    sep  = ", ";
                    term = TRUE;
                }
            } else {
                if (item && nb->fstr) {
                    proto_item_append_text(item, "%s%s", sep, nb->fstr);
                    sep  = ", ";
                    term = TRUE;
                }
            }
            nb++;
        }
        if (term)
            proto_item_append_text(item, ")");

        for (byteno = 0; byteno < len; byteno++) {
            if (bitstring[byteno]) {
                expert_add_info_format(
                    actx->pinfo, item, &ei_ber_bits_unknown,
                    "Unknown bit(s): 0x%s", bytes_to_str(wmem_packet_scope(), bitstring, len));
                break;
            }
        }
    }

    if ((pad > 0) && (pad < 8) && (len > 0)) {
        guint8 bits_in_pad = tvb_get_guint8(tvb, offset + len - 1) & (0xFF >> (8-pad));
        if (bits_in_pad) {
            expert_add_info_format(
                actx->pinfo, item, &ei_ber_bits_set_padded,
                "Bits set in padded area: 0x%02x", bits_in_pad);
        }
    }

    ber_check_length(8*len-pad, min_len, max_len, actx, item, TRUE);

    return end_offset;