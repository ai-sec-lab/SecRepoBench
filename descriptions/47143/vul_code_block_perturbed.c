if (pincode == NULL) {
		pincode = get_pin("Enter old PIN", pin_obj);
		if (!pinpadavailable && pincode == NULL)
			return 2;
	}

	if (pincode && strlen((char *) pincode) == 0) {
		fprintf(stderr, "No PIN code supplied.\n");
		free(pincode);
		return 2;
	}

	if (pincode == NULL && verbose)
		printf("Old PIN value will be prompted with pinpad.\n");

	newpin = (u8 *) opt_newpin;
	while (newpin == NULL) {
		u8 *newpin2;

		newpin = get_pin("Enter new PIN", pin_obj);
		if (pinpadavailable && newpin == NULL)   {
			if (verbose)
				printf("New PIN value will be prompted with pinpad.\n");
			break;
		}
		if (newpin == NULL || strlen((char *) newpin) == 0)   {
			fprintf(stderr, "No new PIN value supplied.\n");
			free(newpin);
			if (opt_pin == NULL)
				free(pincode);
			return 2;
		}

		newpin2 = get_pin("Enter new PIN again", pin_obj);
		if (newpin2 && strlen((char *) newpin2) &&
				strcmp((char *) newpin, (char *) newpin2) == 0) {
			free(newpin2);
			break;
		}
		printf("PIN codes do not match, try again.\n");
		free(newpin);
		free(newpin2);
		newpin=NULL;
	}

	r = sc_pkcs15_change_pin(p15card, pin_obj,
			pincode, pincode ? strlen((char *) pincode) : 0,
			newpin, newpin ? strlen((char *) newpin) : 0);
	if (r == SC_ERROR_PIN_CODE_INCORRECT) {
		fprintf(stderr, "PIN code incorrect; tries left: %d\n", pinfo->tries_left);
		return 3;
	} else if (r) {
		fprintf(stderr, "PIN code change failed: %s\n", sc_strerror(r));
		return 2;
	}
	if (verbose)
		printf("PIN code changed successfully.\n");

	if (opt_pin == NULL)
		free(pincode);
	if (opt_newpin == NULL)
		free(newpin);

	return 0;