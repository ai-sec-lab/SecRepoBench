static int
bootp_option(tvbuff_t *tvb, packet_info *pinfo, proto_tree *bp_tree, int voff,
	     int eoff, gboolean first_pass, gboolean *at_end, const char **dhcp_type_p,
	     const guint8 **vendor_class_id_p, guint8 *overload_p)
{
	struct opt_info *opt;
	guchar		 code = tvb_get_guint8(tvb, voff);
	int		 optlen;
	int		 i, bytesconsumed;
	proto_tree	*v_tree;
	proto_item	*vti, *ti_value;
	tvbuff_t *option_tvb;
	bootp_option_data_t option_data;

	/* Options whose length isn't "optlen + 2". */
	switch (code) {

	case 0:		/* Padding */
		/* check how much padding we have */
		for (i = voff + 1; i < eoff; i++ ) {
			if (tvb_get_guint8(tvb, i) != 0) {
				break;
			}
		}
		i = i - voff;
		if (!first_pass) {
			if (bp_tree != NULL) {
				vti = proto_tree_add_uint_format_value(bp_tree, hf_bootp_option_type,
						tvb, voff, 1, 0, "(0) Padding");
				v_tree = proto_item_add_subtree(vti, ett_bootp_option);
				proto_tree_add_item(v_tree, hf_bootp_option_padding, tvb, voff, i, ENC_NA);
			}
		}
		bytesconsumed = i;
		return bytesconsumed;

	case 255:	/* End Option */
		if (!first_pass) {
			if (bp_tree != NULL) {
				vti = proto_tree_add_uint_format_value(bp_tree, hf_bootp_option_type,
						tvb, voff, 1, 0, "(255) End");
				v_tree = proto_item_add_subtree(vti, ett_bootp_option);
				if (*overload_p)
					proto_tree_add_item(v_tree, hf_bootp_option_end_overload, tvb, voff, 1, ENC_BIG_ENDIAN);
				else
					proto_tree_add_item(v_tree, hf_bootp_option_end, tvb, voff, 1, ENC_BIG_ENDIAN);
			}
		}
		*at_end = TRUE;
		bytesconsumed = 1;
		return bytesconsumed;
	}

	/*
	 * Get the length of the option, and the number of bytes it
	 * consumes (the length doesn't include the option code or
	 * length bytes).
	 *
	 * On the first pass, check first whether we have the length
	 * byte, so that we don't throw an exception; if we throw an
	 * exception in the first pass, which is only checking for options
	 * whose values we need in order to properly dissect the packet
	 * on the second pass, we won't actually dissect the options, so
	 * you won't be able to see which option had the problem.
	 */
	if (first_pass) {
		if (!tvb_bytes_exist(tvb, voff+1, 1)) {
			/*
			 * We don't have the length byte; just return 1
			 * as the number of bytes we consumed, to count
			 * the code byte.
			 */
			return 1;
		}
	}
	optlen = tvb_get_guint8(tvb, voff+1);
	bytesconsumed = optlen + 2;

	/*
	 * In the first pass, we don't put anything into the protocol
	 * tree; we just check for some options we have to look at
	 * in order to properly process the packet:
	 *
	 *	52 (Overload) - we need this to properly dissect the
	 *	   file and sname fields
	 *
	 *	53 (DHCP message type) - if this is present, this is DHCP
	 *
	 *	60 (Vendor class identifier) - we need this in order to
	 *	   interpret the vendor-specific info
	 *
	 * We also check, before fetching anything, to make sure we
	 * have the entire item we're fetching, so that we don't throw
	 * an exception.
	 */
	if (first_pass) {
		if (tvb_bytes_exist(tvb, voff+2, bytesconsumed-2)) {
			switch (code) {

			case 52:
				*overload_p = tvb_get_guint8(tvb, voff+2);
				break;

			case 53:
				*dhcp_type_p =
				    val_to_str(tvb_get_guint8(tvb, voff+2),
					opt53_text,
					"Unknown Message Type (0x%02x)");
				break;

			case 60:
				// <MASK>
				break;
			case 119:
				rfc3396_dns_domain_search_list.total_number_of_block++;
				break;
			case 120:
				rfc3396_sip_server.total_number_of_block++;
				break;
			}
		}

		/*
		 * We don't do anything else here.
		 */
		return bytesconsumed;
	}

	/* Normal cases */
	opt = bootp_get_opt(code);
	if (opt == NULL)
	{
		/* THIS SHOULD NEVER HAPPEN!!! */
		return bytesconsumed;
	}

	vti = proto_tree_add_uint_format_value(bp_tree, hf_bootp_option_type,
		tvb, voff, bytesconsumed, code, "(%d) %s", code, opt->text);
	v_tree = proto_item_add_subtree(vti, ett_bootp_option);
	proto_tree_add_item(v_tree, hf_bootp_option_length, tvb, voff+1, 1, ENC_BIG_ENDIAN);

	ti_value = proto_tree_add_item(v_tree, hf_bootp_option_value, tvb, voff+2, optlen, ENC_NA);
	PROTO_ITEM_SET_HIDDEN(ti_value);

	/* prepate data for dissector table */
	option_tvb = tvb_new_subset_length(tvb, voff+2, optlen);
	option_data.option = code;
	option_data.overload = overload_p;
	option_data.dhcp_type = *dhcp_type_p;
	option_data.vendor_class_id = *vendor_class_id_p;
	option_data.orig_tvb = tvb;

	if (!dissector_try_uint_new(bootp_option_table, code, option_tvb, pinfo, v_tree, FALSE, &option_data)) {
		/* hf_bootp_option_value is already in tree, just make it visible */
		PROTO_ITEM_SET_VISIBLE(ti_value);
	}

	return bytesconsumed;
}