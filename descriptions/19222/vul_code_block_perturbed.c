if ((attr_type == CKA_CLASS) || (attr_type == CKA_CERTIFICATE_TYPE)
									 || (attr_type == CKA_KEY_TYPE)) {
		if (len != 4) {
			return SC_ERROR_CORRUPTED_DATA;
		}
		attr_out->attribute_data_type = SC_CARDCTL_COOLKEY_ATTR_TYPE_ULONG;
	}
	/* return the length and the data */
	attr_out->attribute_length = len;
	attr_out->attribute_value = attributedata+sizeof(coolkey_v0_attribute_header_t);