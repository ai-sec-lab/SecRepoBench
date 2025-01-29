if (r < 2 || buf[0] != 0xA0)
				continue;
			for (i = 2; i < buf[1] + 2 && (i + 2) < r; i += 2 + buf[i + 1]) {
				if (buf[i] == 0x83 && buf[i + 1] == 1 && buf[i + 2] == pin_reference) {
					++found;
				}
				if (buf[i] == 0x90 && (i + 1 + buf[i + 1]) < r) {
					fbz = buf[i + 1 + buf[i + 1]];
				}
			}
			if (found) {
				pin_info.tries_left = fbz;
				break;
			}