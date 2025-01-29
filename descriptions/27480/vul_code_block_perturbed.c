int matchfound = 0;
			if (buf[0] != 0xA0)
				continue;
			for (i = 2; i < buf[1] + 2; i += 2 + buf[i + 1]) {
				if (buf[i] == 0x83 && buf[i + 1] == 1 && buf[i + 2] == key_reference)
					++matchfound;
			}