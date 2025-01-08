
			if (!skip_logs) {
				GF_LOG(GF_LOG_WARNING, GF_LOG_CONTAINER, ("[iso file] Box \"%s\" is larger than container box\n", gf_4cc_to_str(a->type)));
			}
			if (!gf_opts_get_bool("core", "no-check")) {
				gf_isom_box_del(a);
				return GF_ISOM_INVALID_FILE;
			}
			parent->size = 0;
		