static fragment_head *
fragment_add_seq_single_work(reassembly_table *table, tvbuff_t *tvb,
			     const int offset, const packet_info *pinfo,
		             const guint32 id, const void* data,
			     const guint32 frag_data_len,
			     const gboolean first, const gboolean last,
			     const guint32 max_frags, const guint32 max_age,
			     const guint32 flags)
{
	reassembled_key reass_key;
	tvbuff_t *old_tvb_data;
	gpointer orig_key;
	fragment_head *fh, *new_fh;
	fragment_item *fd, *prev_fd;
	guint32 frag_number, tmp_offset;
	/* Have we already seen this frame?
	 * If so, look for it in the table of reassembled packets.
	 * Note here we store in the reassembly table by the single sequence
	 * number rather than the sequence number of the First fragment. */
	if (pinfo->fd->visited) {
		reass_key.frame = pinfo->num;
		reass_key.id = id;
		fh = (fragment_head *)g_hash_table_lookup(table->reassembled_table, &reass_key);
		return fh;
	}
	/* First let's figure out where we want to add our new fragment */
	fh = NULL;
	// <MASK>
}