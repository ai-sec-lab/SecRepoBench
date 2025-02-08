static int
pgp_parse_algo_attr_blob(const pgp_blob_t *blob, sc_cardctl_openpgp_keygen_info_t *key_info)
{
	struct sc_object_id oid;
	unsigned int index;

	if (blob == NULL || blob->data == NULL || blob->len == 0 ||
	    blob->id < 0x00c1 || blob->id > 0x00c3 || key_info == NULL)
		return SC_ERROR_INCORRECT_PARAMETERS;

	key_info->key_id = blob->id - 0x00c0;	/* attribute algorithm blobs are C1 - C3 */

	switch (blob->data[0]) {
		case SC_OPENPGP_KEYALGO_RSA:
			if (blob->len < 5)
				return SC_ERROR_INCORRECT_PARAMETERS;

			key_info->algorithm = SC_OPENPGP_KEYALGO_RSA;
			key_info->u.rsa.modulus_len = bebytes2ushort(blob->data + 1);
			key_info->u.rsa.exponent_len = bebytes2ushort(blob->data + 3);

			key_info->u.rsa.keyformat = (blob->len > 5)
						  ? blob->data[5]
						  : SC_OPENPGP_KEYFORMAT_RSA_STD;
			break;
		case SC_OPENPGP_KEYALGO_ECDH:
		case SC_OPENPGP_KEYALGO_ECDSA:

			/* SC_OPENPGP_KEYALGO_ECDH || SC_OPENPGP_KEYALGO_ECDSA */
			// <MASK>
			for (index=0; ec_curves[index].oid.value[0] >= 0; index++){
				if (sc_compare_oid(&ec_curves[index].oid_binary, &oid)){
					key_info->u.ec.oid = ec_curves[index].oid;
					key_info->u.ec.key_length = ec_curves[index].size;
					break;
				}
			}
			break;
		default:
			return SC_ERROR_NOT_IMPLEMENTED;
	}

	return SC_SUCCESS;
}