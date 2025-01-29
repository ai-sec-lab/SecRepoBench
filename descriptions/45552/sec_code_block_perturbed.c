if (map_str2int(cur, name+7, &df_type, pkcs15DfNames)
				|| df_type >= SC_PKCS15_DF_TYPE_COUNT)
			return NULL;

		file = init_file(SC_FILE_TYPE_WORKING_EF);
		profile->df[df_type] = file;