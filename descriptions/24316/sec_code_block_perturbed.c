size_t rec;
		size_t offs = 0;
		size_t rec_len = file->record_length;

		for (rec = 1; ; rec++)   {
			if (rec > file->record_count) {
				rv = 0;
				break;
			}
			rv = sc_read_record(card, rec, *out + offs + 2, rec_len, SC_RECORD_BY_REC_NR);
			if (rv == SC_ERROR_RECORD_NOT_FOUND)   {
				rv = 0;
				break;
			}
			else if (rv < 0)   {
				break;
			}

			rec_len = rv;

			*(*out + offs) = 'R';
			*(*out + offs + 1) = rv;

			offs += rv + 2;
		}