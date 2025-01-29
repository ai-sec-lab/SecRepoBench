key_decoded = get_utf_8_string(pinfo->pool, key, next_offset - offset);
		proto_tree_add_string(sub, hf_form_key, tvb, offset, next_offset - offset, key_decoded);
		proto_item_append_text(sub, ": \"%s\"", format_text(pinfo->pool, key, strlen(key)));

		offset = next_offset+1;

		next_offset = get_form_key_value(tvb, &value, offset, '&');
		if (next_offset == -1)
			break;
		value_decoded = get_utf_8_string(pinfo->pool, value, next_offset - offset);