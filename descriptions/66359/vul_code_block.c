
			char *frame = dcfg;
			while ((i+3<vosh_size)  && ((frame[i]!=0) || (frame[i+1]!=0) || (frame[i+2]!=1))) i++;
			if (i+4>=vosh_size) break;
			if (strncmp(frame+i+4, "DivX", 4)) {
				i += 4;
				continue;
			}
			frame = memchr(dcfg + i + 4, 'p', vosh_size - i - 4);
			if (frame) {
				ctx->forced_packed = GF_TRUE;
				frame[0] = 'n';
			}
			break;
		