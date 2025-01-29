p->value[p->len++] = (u8) (file->id / 256);
	p->value[p->len++] = (u8) (file->id % 256);

	/* Increment FID until there's no file with such path */
	r = sc_select_file(card, p, NULL);
	while(r == 0) {
		file->id++;
		p->value[p->len - 2] = (u8) (file->id / 256);
		p->value[p->len - 1] = (u8) (file->id % 256);
		r = sc_select_file(card, p, NULL);
	}

	*out = file;
	return 0;