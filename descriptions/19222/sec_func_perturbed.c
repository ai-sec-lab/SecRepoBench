static int
coolkey_v0_get_attribute_data(const u8 *attributedata, size_t buf_len, sc_cardctl_coolkey_attribute_t *attr_out)
{
	/* we need to manually detect types CK_ULONG */
	CK_ATTRIBUTE_TYPE attr_type = coolkey_get_attribute_type(attributedata, COOLKEY_V0_OBJECT, buf_len);
	int r;
	size_t len;

	attr_out->attribute_data_type = SC_CARDCTL_COOLKEY_ATTR_TYPE_STRING;
	attr_out->attribute_length = 0;
	attr_out->attribute_value = NULL;

	r = coolkey_v0_get_attribute_len(attributedata, buf_len, &len);
	if (r < 0) {
		return r;
	}
	if (len + sizeof(coolkey_v0_attribute_header_t) > buf_len) {
		return SC_ERROR_CORRUPTED_DATA;
	}
	if ((attr_type == CKA_CLASS) || (attr_type == CKA_CERTIFICATE_TYPE)
									 || (attr_type == CKA_KEY_TYPE)) {
		if (len != 4) {
			return SC_ERROR_CORRUPTED_DATA;
		}
		attr_out->attribute_data_type = SC_CARDCTL_COOLKEY_ATTR_TYPE_ULONG;
	}
	/* return the length and the data */
	attr_out->attribute_length = len;
	attr_out->attribute_value = attributedata + sizeof(coolkey_v0_attribute_header_t);
	return SC_SUCCESS;
}