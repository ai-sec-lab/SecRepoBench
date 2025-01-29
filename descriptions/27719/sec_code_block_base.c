if (r <= 0) {
			sc_log(ctx, "No EF_KEYD-Record found\n");
			return 1;
		}
		for (i = 0; i + 1 < r; i += 2 + buf[i + 1]) {
			if (buf[i] == 0xB6)
				can_sign++;
			if (buf[i] == 0xB8)
				can_crypt++;
		}