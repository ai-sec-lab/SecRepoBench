static int change_pin(void)
{
	sc_pkcs15_object_t *pin_obj;
	sc_pkcs15_auth_info_t *pinfo = NULL;
	u8 *pincode, *newpin;
	int r, pinpadavailable = 0;

	pinpadavailable = p15card->card->reader->capabilities & SC_READER_CAP_PIN_PAD
	   	|| p15card->card->caps & SC_CARD_CAP_PROTECTED_AUTHENTICATION_PATH;

	if (!(pin_obj = get_pin_info()))
		return 2;

	pinfo = (sc_pkcs15_auth_info_t *) pin_obj->data;
	if (pinfo->auth_type != SC_PKCS15_PIN_AUTH_TYPE_PIN)
		return 1;

	if (pinfo->tries_left != -1) {
		if (pinfo->tries_left != pinfo->max_tries) {
			if (pinfo->tries_left == 0) {
				fprintf(stderr, "PIN code blocked!\n");
				return 2;
			} else {
				fprintf(stderr, "%d PIN tries left.\n", pinfo->tries_left);
			}
		}
	}

	pincode = (u8 *) opt_pin;
	// <MASK>
}