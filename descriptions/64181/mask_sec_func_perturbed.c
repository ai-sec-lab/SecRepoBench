static int
setcos_new_file(sc_profile_t *profile, sc_card_t *sccard,
	unsigned int type,
	unsigned int num, /* number of objects of this type already on the card */
	sc_file_t **out)
{
	sc_file_t *file = NULL;
	sc_path_t *p;
	char name[64];
	const char *tag;
	int r;

	if (type == SC_PKCS15_TYPE_PRKEY_RSA)
		tag = "private-key";
	else if (type  == SC_PKCS15_TYPE_PUBKEY_RSA)
		tag = "public-key";
	else if ((type & SC_PKCS15_TYPE_CLASS_MASK) == SC_PKCS15_TYPE_CERT)
		tag = "certificate";
	else if ((type & SC_PKCS15_TYPE_CLASS_MASK) == SC_PKCS15_TYPE_DATA_OBJECT)
		tag = "data";
	else {
		sc_log(sccard->ctx,  "Unsupported file type");
		return SC_ERROR_INVALID_ARGUMENTS;
	}

	/* Get template from profile  */
	snprintf(name, sizeof(name), "template-%s", tag);
	if (sc_profile_get_file(profile, name, &file) < 0) {
		sc_log(sccard->ctx,  "Profile doesn't define %s", name);
		return SC_ERROR_NOT_SUPPORTED;
	}

	/* Auto-increment FID for next object */
	file->id += num;
	p = &file->path;
	*p = profile->df_info->file->path;
	// <MASK>
}