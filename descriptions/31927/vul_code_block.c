
			switch (ptr->sampleSize) {
			case 4:
				ptr->sizes[i] = gf_bs_read_int(bs, 4);
				if (i+1 < ptr->sampleCount) {
					ptr->sizes[i+1] = gf_bs_read_int(bs, 4);
				} else {
					//0 padding in odd sample count
					gf_bs_read_int(bs, 4);
				}
				i += 2;
				break;
			default:
				ptr->sizes[i] = gf_bs_read_int(bs, ptr->sampleSize);
				i += 1;
				break;
			}
			if (ptr->max_size < ptr->sizes[i])
				ptr->max_size = ptr->sizes[i];
			ptr->total_size += ptr->sizes[i];
			ptr->total_samples++;
		