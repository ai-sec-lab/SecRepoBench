if (parent->size < a->size) {
			if (!skip_logs) {
				GF_LOG(GF_LOG_WARNING, GF_LOG_CONTAINER, ("[iso file] Box \"%s\" is larger than container box\n", gf_4cc_to_str(a->type)));
			}
			parent->size = 0;
		} else {
			parent->size -= a->size;
		}