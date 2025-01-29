key_info->algorithm = blob->data[0];

			sc_init_oid(&oid);
			/* Create copy of oid from blob */
			for (j=0; j < (blob->len-1); j++) {
				oid.value[j] = blob->data[j+1]; /* ignore first byte of blob (algo ID) */
			}

			/* compare with list of supported ec_curves */