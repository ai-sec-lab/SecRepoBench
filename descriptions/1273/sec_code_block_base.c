*vendor_class_id_p =
					tvb_get_string_enc(wmem_packet_scope(),
					tvb, voff+2, consumed-2, ENC_ASCII);