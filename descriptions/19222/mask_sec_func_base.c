static int
coolkey_v0_get_attribute_data(const u8 *attr, size_t buf_len, sc_cardctl_coolkey_attribute_t *attr_out)
{
	/* we need to manually detect types CK_ULONG */
	CK_ATTRIBUTE_TYPE attr_type = coolkey_get_attribute_type(attr, COOLKEY_V0_OBJECT, buf_len);
	int r;
	size_t len;

	attr_out->attribute_data_type = SC_CARDCTL_COOLKEY_ATTR_TYPE_STRING;
	attr_out->attribute_length = 0;
	attr_out->attribute_value = NULL;

	r = coolkey_v0_get_attribute_len(attr, buf_len, &len);
	if (r < 0) {
		return r;
	}
	// <MASK>
	return SC_SUCCESS;
}