GF_Err iloc_box_read(GF_Box *s, GF_BitStream *bs)
{
	u32 item_count, extent_count, i, j;
	GF_ItemLocationBox *ptr = (GF_ItemLocationBox *)s;

	ISOM_DECREASE_SIZE(ptr, 2)
	ptr->offset_size = gf_bs_read_int(bs, 4);
	ptr->length_size = gf_bs_read_int(bs, 4);
	ptr->base_offset_size = gf_bs_read_int(bs, 4);
	if (ptr->version == 1 || ptr->version == 2) {
		ptr->index_size = gf_bs_read_int(bs, 4);
	} else {
		gf_bs_read_int(bs, 4);
	}
	if (ptr->version < 2) {
		ISOM_DECREASE_SIZE(ptr, 2)
		item_count = gf_bs_read_u16(bs);
	} else {
		ISOM_DECREASE_SIZE(ptr, 4)
		item_count = gf_bs_read_u32(bs);
	}

	for (i = 0; i < item_count; i++) {
		// <MASK>
	}
	return GF_OK;
}