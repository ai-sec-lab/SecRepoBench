unsigned char flg;
	size_t data_start = 10;

	if (*n < 4) {
		goto err;	
	}

	flg = old[3];