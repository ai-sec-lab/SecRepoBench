case 0x01: /* Continuation fragment of an L2CAP message, or an Empty PDU */
            if (length > 0) {
                tvbuff_t *new_tvb = NULL;

                pinfo->fragmented = TRUE;
                if (connection_info && !retransmit) {
                    if (!pinfo->fd->visited) {
                        if (connection_info->direction_info[direction].segmentation_started == 1) {
                            if (connection_info->direction_info[direction].segment_len_rem >= length) {
                                connection_info->direction_info[direction].segment_len_rem = connection_info->direction_info[direction].segment_len_rem - length;
                            } else {
                                /*
                                 * Missing fragment for previous L2CAP and fragment start for this.
                                 * Increase l2cap_index.
                                 */
                                btle_frame_info->missing_start = 1;
                                btle_frame_info->l2cap_index = l2cap_index;
                                l2cap_index++;
                            }
                            if (connection_info->direction_info[direction].segment_len_rem > 0) {
                                btle_frame_info->more_fragments = 1;
                            }
                            else {
                                btle_frame_info->more_fragments = 0;
                                connection_info->direction_info[direction].segmentation_started = 0;
                                connection_info->direction_info[direction].segment_len_rem = 0;
                            }
                        } else {
                            /*
                             * Missing fragment start.
                             * Set more_fragments and increase l2cap_index to avoid reassembly.
                             */
                            btle_frame_info->more_fragments = 1;
                            btle_frame_info->missing_start = 1;
                            btle_frame_info->l2cap_index = l2cap_index;
                            l2cap_index++;
                        }
                    }

                    add_l2cap_index = TRUE;

                    frag_btl2cap_msg = fragment_add_seq_next(&btle_l2cap_msg_reassembly_table,
                        tvb, offset,
                        pinfo,
                        btle_frame_info->l2cap_index,      /* guint32 ID for fragments belonging together */
                        NULL,                              /* data* */
                        length,                            /* Fragment length */
                        btle_frame_info->more_fragments);  /* More fragments */

                    new_tvb = process_reassembled_data(tvb, offset, pinfo,
                        "Reassembled L2CAP",
                        frag_btl2cap_msg,
                        &btle_l2cap_msg_frag_items,
                        NULL,
                        btle_tree);
                }

                if (new_tvb) {
                    bthci_acl_data_t  *acl_data;

                    col_set_str(pinfo->cinfo, COL_INFO, "L2CAP Data");

                    acl_data = wmem_new(wmem_packet_scope(), bthci_acl_data_t);
                    acl_data->interface_id = interface_id;
                    acl_data->adapter_id = adapter_id;
                    acl_data->chandle = 0; /* No connection handle at this layer */
                    acl_data->remote_bd_addr_oui = 0;
                    acl_data->remote_bd_addr_id = 0;
                    acl_data->is_btle = TRUE;
                    acl_data->is_btle_retransmit = retransmit;

                    next_tvb = tvb_new_subset_length(tvb, offset, length);
                    if (next_tvb) {
                        call_dissector_with_data(btl2cap_handle, new_tvb, pinfo, tree, acl_data);
                    }
                    offset += length;
                }
                else {
                    col_set_str(pinfo->cinfo, COL_INFO, "L2CAP Fragment");
                    item = proto_tree_add_item(btle_tree, hf_l2cap_fragment, tvb, offset, length, ENC_NA);
                    if (btle_frame_info->missing_start) {
                        expert_add_info(pinfo, item, &ei_missing_fragment_start);
                    }
                    offset += length;
                }
            } else {
                col_set_str(pinfo->cinfo, COL_INFO, "Empty PDU");
            }

            break;
        case 0x02: /* Start of an L2CAP message or a complete L2CAP message with no fragmentation */
            if (length > 0) {
                guint l2cap_len = tvb_get_letohs(tvb, offset);
                if (l2cap_len + 4 > length) { /* L2CAP PDU Length excludes the 4 octets header */
                    pinfo->fragmented = TRUE;
                    if (connection_info && !retransmit) {
                        if (!pinfo->fd->visited) {
                            connection_info->direction_info[direction].segmentation_started = 1;
                            /* The first two octets in the L2CAP PDU contain the length of the entire
                             * L2CAP PDU in octets, excluding the Length and CID fields(4 octets).
                             */
                            connection_info->direction_info[direction].segment_len_rem = l2cap_len + 4 - length;
                            connection_info->direction_info[direction].l2cap_index = l2cap_index;
                            btle_frame_info->more_fragments = 1;
                            btle_frame_info->l2cap_index = l2cap_index;
                            l2cap_index++;
                        }

                        add_l2cap_index = TRUE;

                        frag_btl2cap_msg = fragment_add_seq_next(&btle_l2cap_msg_reassembly_table,
                            tvb, offset,
                            pinfo,
                            btle_frame_info->l2cap_index,      /* guint32 ID for fragments belonging together */
                            NULL,                              /* data* */
                            length,                            /* Fragment length */
                            btle_frame_info->more_fragments);  /* More fragments */

                        process_reassembled_data(tvb, offset, pinfo,
                            "Reassembled L2CAP",
                            frag_btl2cap_msg,
                            &btle_l2cap_msg_frag_items,
                            NULL,
                            btle_tree);
                    }

                    col_set_str(pinfo->cinfo, COL_INFO, "L2CAP Fragment Start");
                    proto_tree_add_item(btle_tree, hf_l2cap_fragment, tvb, offset, length, ENC_NA);
                    offset += length;
                } else {
                    bthci_acl_data_t  *acl_data;
                    if (connection_info) {
                        /* Add a L2CAP index for completeness */
                        if (!pinfo->fd->visited) {
                            btle_frame_info->l2cap_index = l2cap_index;
                            l2cap_index++;
                        }

                        add_l2cap_index = TRUE;
                    }

                    col_set_str(pinfo->cinfo, COL_INFO, "L2CAP Data");

                    acl_data = wmem_new(wmem_packet_scope(), bthci_acl_data_t);
                    acl_data->interface_id = interface_id;
                    acl_data->adapter_id   = adapter_id;
                    acl_data->chandle      = 0; /* No connection handle at this layer */
                    acl_data->remote_bd_addr_oui = 0;
                    acl_data->remote_bd_addr_id  = 0;
                    acl_data->is_btle = TRUE;
                    acl_data->is_btle_retransmit = retransmit;

                    next_tvb = tvb_new_subset_length(tvb, offset, length);
                    call_dissector_with_data(btl2cap_handle, next_tvb, pinfo, tree, acl_data);
                    offset += length;
                }
            }
            break;
        case 0x03: /* Control PDU */
            proto_tree_add_item(btle_tree, hf_control_opcode, tvb, offset, 1, ENC_LITTLE_ENDIAN);
            control_opcode = tvb_get_guint8(tvb, offset);
            offset += 1;

            col_add_fstr(pinfo->cinfo, COL_INFO, "Control Opcode: %s",
                    val_to_str_ext_const(control_opcode, &control_opcode_vals_ext, "Unknown"));

            switch (control_opcode) {
            case 0x05: /* LL_START_ENC_REQ */
            case 0x06: /* LL_START_ENC_RSP */
            case 0x0A: /* LL_PAUSE_ENC_REQ */
            case 0x0B: /* LL_PAUSE_ENC_RSP */
            case 0x12: /* LL_PING_REQ */
            case 0x13: /* LL_PING_RSP */
                if (tvb_reported_length_remaining(tvb, offset) > 3) {
                    proto_tree_add_expert(btle_tree, pinfo, &ei_unknown_data, tvb, offset, tvb_reported_length_remaining(tvb, offset) - 3);
                    offset += tvb_reported_length_remaining(tvb, offset) - 3;
                }

                break;
            case 0x00: /* LL_CONNECTION_UPDATE_REQ */
                proto_tree_add_item(btle_tree, hf_control_window_size, tvb, offset, 1, ENC_LITTLE_ENDIAN);
                offset += 1;

                proto_tree_add_item(btle_tree, hf_control_window_offset, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                proto_tree_add_item(btle_tree, hf_control_interval, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                proto_tree_add_item(btle_tree, hf_control_latency, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                proto_tree_add_item(btle_tree, hf_control_timeout, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                proto_tree_add_item(btle_tree, hf_control_instant, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                break;
            case 0x01: /* LL_CHANNEL_MAP_REQ */
                sub_item = proto_tree_add_item(btle_tree, hf_control_channel_map, tvb, offset, 5, ENC_NA);
                sub_tree = proto_item_add_subtree(sub_item, ett_channel_map);

                call_dissector(btcommon_le_channel_map_handle, tvb_new_subset_length(tvb, offset, 5), pinfo, sub_tree);
                offset += 5;

                proto_tree_add_item(btle_tree, hf_control_instant, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                break;
            case 0x02: /* LL_TERMINATE_IND */
            case 0x0D: /* LL_REJECT_IND */
                proto_tree_add_item(btle_tree, hf_control_error_code, tvb, offset, 1, ENC_LITTLE_ENDIAN);
                offset += 1;

                break;
            case 0x03: /* LL_ENC_REQ */
                proto_tree_add_item(btle_tree, hf_control_random_number, tvb, offset, 8, ENC_LITTLE_ENDIAN);
                offset += 8;

                proto_tree_add_item(btle_tree, hf_control_encrypted_diversifier, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                proto_tree_add_item(btle_tree, hf_control_master_session_key_diversifier, tvb, offset, 8, ENC_LITTLE_ENDIAN);
                offset += 8;

                proto_tree_add_item(btle_tree, hf_control_master_session_initialization_vector, tvb, offset, 4, ENC_LITTLE_ENDIAN);
                offset += 4;

                break;
            case 0x04: /* LL_ENC_RSP */
                proto_tree_add_item(btle_tree, hf_control_slave_session_key_diversifier, tvb, offset, 8, ENC_LITTLE_ENDIAN);
                offset += 8;

                proto_tree_add_item(btle_tree, hf_control_slave_session_initialization_vector, tvb, offset, 4, ENC_LITTLE_ENDIAN);
                offset += 4;

                break;
            case 0x07: /* LL_UNKNOWN_RSP */
                proto_tree_add_item(btle_tree, hf_control_unknown_type, tvb, offset, 1, ENC_LITTLE_ENDIAN);
                offset += 1;

                break;
            case 0x08: /* LL_FEATURE_REQ */
            case 0x09: /* LL_FEATURE_RSP */
            case 0x0E: /* LL_SLAVE_FEATURE_REQ */
                sub_item = proto_tree_add_item(btle_tree, hf_control_feature_set, tvb, offset, 8, ENC_LITTLE_ENDIAN);
                sub_tree = proto_item_add_subtree(sub_item, ett_features);

                proto_tree_add_bitmask_list(sub_tree, tvb, offset, 1, hfx_control_feature_set_1, ENC_NA);
                offset += 1;

                proto_tree_add_bitmask_list(sub_tree, tvb, offset, 1, hfx_control_feature_set_2, ENC_NA);
                offset += 1;

                proto_tree_add_bitmask_list(sub_tree, tvb, offset, 1, hfx_control_feature_set_3, ENC_NA);
                offset += 1;

                proto_tree_add_item(sub_tree, hf_control_feature_set_reserved, tvb, offset, 5, ENC_NA);
                offset += 5;

                break;
            case 0x0C: /* LL_VERSION_IND */
                proto_tree_add_item(btle_tree, hf_control_version_number, tvb, offset, 1, ENC_LITTLE_ENDIAN);
                offset += 1;

                proto_tree_add_item(btle_tree, hf_control_company_id, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                proto_tree_add_item(btle_tree, hf_control_subversion_number, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                break;
            case 0x0F: /* LL_CONNECTION_PARAM_REQ */
            case 0x10: /* LL_CONNECTION_PARAM_RSP */
                proto_tree_add_item(btle_tree, hf_control_interval_min, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                proto_tree_add_item(btle_tree, hf_control_interval_max, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                proto_tree_add_item(btle_tree, hf_control_latency, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                proto_tree_add_item(btle_tree, hf_control_timeout, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                proto_tree_add_item(btle_tree, hf_control_preferred_periodicity, tvb, offset, 1, ENC_LITTLE_ENDIAN);
                offset += 1;

                proto_tree_add_item(btle_tree, hf_control_reference_connection_event_count, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                proto_tree_add_item(btle_tree, hf_control_offset_0, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                proto_tree_add_item(btle_tree, hf_control_offset_1, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                proto_tree_add_item(btle_tree, hf_control_offset_2, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                proto_tree_add_item(btle_tree, hf_control_offset_3, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                proto_tree_add_item(btle_tree, hf_control_offset_4, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                proto_tree_add_item(btle_tree, hf_control_offset_5, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                break;
            case 0x11: /* LL_REJECT_IND_EXT */
                proto_tree_add_item(btle_tree, hf_control_reject_opcode, tvb, offset, 1, ENC_LITTLE_ENDIAN);
                offset += 1;

                proto_tree_add_item(btle_tree, hf_control_error_code, tvb, offset, 1, ENC_LITTLE_ENDIAN);
                offset += 1;

                break;
            case 0x14: /* LL_LENGTH_REQ */
            case 0x15: /* LL_LENGTH_RSP */
                proto_tree_add_item(btle_tree, hf_control_max_rx_octets, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                proto_tree_add_item(btle_tree, hf_control_max_rx_time, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                proto_tree_add_item(btle_tree, hf_control_max_tx_octets, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                proto_tree_add_item(btle_tree, hf_control_max_tx_time, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                break;
            case 0x16: /* LL_PHY_REQ */
            case 0x17: /* LL_PHY_RSP */
                proto_tree_add_bitmask(btle_tree, tvb, offset, hf_control_tx_phys, ett_tx_phys, hfx_control_phys_sender, ENC_NA);
                offset += 1;

                proto_tree_add_bitmask(btle_tree, tvb, offset, hf_control_rx_phys, ett_rx_phys, hfx_control_phys_sender, ENC_NA);
                offset += 1;

                break;
            case 0x18: /* LL_PHY_UPDATE_IND */
                proto_tree_add_bitmask(btle_tree, tvb, offset, hf_control_m_to_s_phy, ett_m_to_s_phy, hfx_control_phys_update, ENC_NA);
                offset += 1;

                proto_tree_add_bitmask(btle_tree, tvb, offset, hf_control_s_to_m_phy, ett_s_to_m_phy, hfx_control_phys_update, ENC_NA);
                offset += 1;

                proto_tree_add_item(btle_tree, hf_control_instant, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                break;
            case 0x19: /* LL_MIN_USED_CHANNELS_IND */
                proto_tree_add_bitmask(btle_tree, tvb, offset, hf_control_phys, ett_phys, hfx_control_phys, ENC_NA);
                offset += 1;

                proto_tree_add_item(btle_tree, hf_control_min_used_channels, tvb, offset, 1, ENC_LITTLE_ENDIAN);
                offset += 1;

                break;
            default:
                if (tvb_reported_length_remaining(tvb, offset) > 3) {
                    proto_tree_add_expert(btle_tree, pinfo, &ei_unknown_data, tvb, offset, tvb_reported_length_remaining(tvb, offset) - 3);
                    offset += tvb_reported_length_remaining(tvb, offset) - 3;
                }
            }

            break;
        default:
            if (tvb_reported_length_remaining(tvb, offset) > 3) {
                proto_tree_add_expert(btle_tree, pinfo, &ei_unknown_data, tvb, offset, tvb_reported_length_remaining(tvb, offset) - 3);
                offset += tvb_reported_length_remaining(tvb, offset) - 3;
            }