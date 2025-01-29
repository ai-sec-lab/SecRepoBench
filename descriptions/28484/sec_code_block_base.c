case '\\':
			c++;
			if (path[c] == '\0') {
				if (!fc->meta) {
					fc->stem_len++;
				}
				fc->str_len++;
				return;
			}