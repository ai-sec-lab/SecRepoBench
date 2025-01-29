switch (ptr->sampleSize) {
			case 4:
				ptr->sizes[index] = gf_bs_read_int(bs, 4);
				if (index+1 < ptr->sampleCount) {
					ptr->sizes[index+1] = gf_bs_read_int(bs, 4);
				} else {
					//0 padding in odd sample count
					gf_bs_read_int(bs, 4);
				}
				index += 2;
				break;
			default:
				ptr->sizes[index] = gf_bs_read_int(bs, ptr->sampleSize);
				index += 1;
				break;
			}
			if (ptr->max_size < ptr->sizes[index])
				ptr->max_size = ptr->sizes[index];
			ptr->total_size += ptr->sizes[index];
			ptr->total_samples++;