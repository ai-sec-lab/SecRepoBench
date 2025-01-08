if (i+8 < vosh_size && strncmp(frame+i+4, "DivX", 4)) {
				i += 4;
				continue;
			}