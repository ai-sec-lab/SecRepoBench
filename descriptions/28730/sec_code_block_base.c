if(idepar <= IDNO_VOID_AREA) {
			if(idepar == 0) {
				proto_tree_add_item(gts_tree, idn_dmx_void, tvb, offset, 1, ENC_BIG_ENDIAN);
				offset += 1;
				dictionary_size += 1;
				if(!words_found)
					i -= 1;
			}else {
				offset = dissect_idn_dmx_gts(tvb, offset, gts_tree, idn_dmx_unknown, NULL);
				for(j=1; j<=idepar; j++) {
					proto_tree_add_item(gts_tree, idn_octet, tvb, offset, 1, ENC_BIG_ENDIAN);
					offset += 1;
					dictionary_size += 1;
					if(words_found)
						i += 1;
				}
				if(!words_found)
					i -= 1;
			}
		}else if(idepar >= IDNTAG_DIMMER_START && idepar <= IDNTAG_DIMMER_END) {
			offset = dissect_idn_dimmer_level_subset(tvb, offset, gts_tree, config, i, &dictionary_size);
		}else {
			offset = dissect_idn_dmx_gts(tvb, offset, gts_tree, idn_dmx_unknown, &dictionary_size);
		}

		if(i == config->word_count && !words_found) {
			curr_size = dictionary_size;
			while(curr_size%4 != 0 && i > 0) {
				i -= 1;
				curr_size += 1;
			}
			words_found = 1;
		}