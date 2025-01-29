if (directorypath.len > SC_MAX_PATH_SIZE)
			return SC_ERROR_INTERNAL;

		nfids = r / 2;
		while (r >= 0 && nfids--) {
			directorypath.value[directorypath.len-2] = buffer[2*nfids];
			directorypath.value[directorypath.len-1] = buffer[2*nfids+1];
			r = sc_select_file(p15card->card, &directorypath, &file);
			if (r < 0) {
				if (r == SC_ERROR_FILE_NOT_FOUND)
					continue;
				break;
			}
			r = sc_pkcs15init_rmdir(p15card, profile, file);
			sc_file_free(file);
		}

		if (r < 0)
			return r;