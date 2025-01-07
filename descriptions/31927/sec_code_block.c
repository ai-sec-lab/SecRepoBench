
			u32 s_size;
			switch (ptr->sampleSize) {
			case 4:
				s_size = ptr->sizes[i] = gf_bs_read_int(bs, 4);
				if (ptr->max_size < s_size)
					ptr->max_size = s_size;
				ptr->total_size += s_size;
				ptr->total_samples++;
				if (i+1 < ptr->sampleCount) {
					s_size = ptr->sizes[i+1] = gf_bs_read_int(bs, 4);
					if (ptr->max_size < s_size)
						ptr->max_size = s_size;
					ptr->total_size += s_size;
					ptr->total_samples++;
				} else {
					//0 padding in odd sample count
					gf_bs_read_int(bs, 4);
				}
				i += 2;
				break;
			default:
				s_size = ptr->sizes[i] = gf_bs_read_int(bs, ptr->sampleSize);
				if (ptr->max_size < s_size)
					ptr->max_size = s_size;
				ptr->total_size += s_size;
				ptr->total_samples++;
				i += 1;
				break;
			}
		