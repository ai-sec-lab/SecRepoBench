u32 s_size;
			switch (ptr->sampleSize) {
			case 4:
				s_size = ptr->sizes[index] = gf_bs_read_int(bs, 4);
				if (ptr->max_size < s_size)
					ptr->max_size = s_size;
				ptr->total_size += s_size;
				ptr->total_samples++;
				if (index+1 < ptr->sampleCount) {
					s_size = ptr->sizes[index+1] = gf_bs_read_int(bs, 4);
					if (ptr->max_size < s_size)
						ptr->max_size = s_size;
					ptr->total_size += s_size;
					ptr->total_samples++;
				} else {
					//0 padding in odd sample count
					gf_bs_read_int(bs, 4);
				}
				index += 2;
				break;
			default:
				s_size = ptr->sizes[index] = gf_bs_read_int(bs, ptr->sampleSize);
				if (ptr->max_size < s_size)
					ptr->max_size = s_size;
				ptr->total_size += s_size;
				ptr->total_samples++;
				index += 1;
				break;
			}