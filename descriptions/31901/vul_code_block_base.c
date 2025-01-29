GF_ItemLocationEntry *location_entry = (GF_ItemLocationEntry *)gf_malloc(sizeof(GF_ItemLocationEntry));
		if (!location_entry) return GF_OUT_OF_MEM;

		gf_list_add(ptr->location_entries, location_entry);
		if (ptr->version < 2) {
			ISOM_DECREASE_SIZE(ptr, 2)
			location_entry->item_ID = gf_bs_read_u16(bs);
		} else {
			ISOM_DECREASE_SIZE(ptr, 4)
			location_entry->item_ID = gf_bs_read_u32(bs);
		}
		if (ptr->version == 1 || ptr->version == 2) {
			ISOM_DECREASE_SIZE(ptr, 2)
			location_entry->construction_method = gf_bs_read_u16(bs);
		}
		else {
			location_entry->construction_method = 0;
		}
		ISOM_DECREASE_SIZE(ptr, (2 + ptr->base_offset_size) )
		location_entry->data_reference_index = gf_bs_read_u16(bs);
		location_entry->base_offset = gf_bs_read_int(bs, 8*ptr->base_offset_size);
#ifndef GPAC_DISABLE_ISOM_WRITE
		location_entry->original_base_offset = location_entry->base_offset;
#endif

		ISOM_DECREASE_SIZE(ptr, 2)
		extent_count = gf_bs_read_u16(bs);
		location_entry->extent_entries = gf_list_new();
		for (j = 0; j < extent_count; j++) {
			GF_ItemExtentEntry *extent_entry = (GF_ItemExtentEntry *)gf_malloc(sizeof(GF_ItemExtentEntry));
			if (!extent_entry) return GF_OUT_OF_MEM;
			
			gf_list_add(location_entry->extent_entries, extent_entry);
			if ((ptr->version == 1 || ptr->version == 2) && ptr->index_size > 0) {
				ISOM_DECREASE_SIZE(ptr, ptr->index_size)
				extent_entry->extent_index = gf_bs_read_int(bs, 8 * ptr->index_size);
			}
			else {
				extent_entry->extent_index = 0;
			}
			ISOM_DECREASE_SIZE(ptr, (ptr->offset_size+ptr->length_size) )

			extent_entry->extent_offset = gf_bs_read_int(bs, 8*ptr->offset_size);
			extent_entry->extent_length = gf_bs_read_int(bs, 8*ptr->length_size);
#ifndef GPAC_DISABLE_ISOM_WRITE
			extent_entry->original_extent_offset = extent_entry->extent_offset;
#endif
		}