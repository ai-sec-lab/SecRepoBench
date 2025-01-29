key_info->algorithm = blob->data[0];

			sc_init_oid(&oid);
			/* Create copy of oid from blob */
			for (index=0; index < (blob->len-1); index++) {
				oid.value[index] = blob->data[index+1]; /* ignore first byte of blob (algo ID) */
			}

			/* compare with list of supported ec_curves */