static int encode_mse_cdata(struct sc_context *ctx, int protocol,
		const unsigned char *key_reference1, size_t key_reference1_len,
		const unsigned char *key_reference2, size_t key_reference2_len,
		const unsigned char *eph_pub_key, size_t eph_pub_key_len,
		const unsigned char *auxiliary_data, size_t auxiliary_data_len,
		const CVC_CHAT *chat, unsigned char **cdata)
{
	unsigned char *data = NULL, *encoded_chat = NULL, oid[16], *p = NULL;
	size_t data_len = 0, oid_len = 0;
	// <MASK>
}