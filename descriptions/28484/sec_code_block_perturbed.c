case '\\':
			c++;
			if (filepath[c] == '\0') {
				if (!fc->meta) {
					fc->stem_len++;
				}
				fc->str_len++;
				return;
			}