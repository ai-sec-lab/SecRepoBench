GF_Err audio_sample_entry_box_read(GF_Box *s, GF_BitStream *bs)
{
	GF_MPEGAudioSampleEntryBox *audioentrybox;
	char *data;
	u8 a, b, c, d;
	u32 i, size, v, nb_alnum;
	GF_Err e;
	u64 pos, start;

	audioentrybox = (GF_MPEGAudioSampleEntryBox *)s;

	start = gf_bs_get_position(bs);
	v = gf_bs_peek_bits(bs, 16, 8);
	if (v)
		audioentrybox->qtff_mode = GF_ISOM_AUDIO_QTFF_ON_NOEXT;

	//try to disambiguate QTFF v1 and MP4 v1 audio sample entries ...
	if (v==1) {
		//go to end of ISOM audio sample entry, skip 4 byte (box size field), read 4 bytes (box type) and check if this looks like a box
		gf_bs_skip_bytes(bs, 8 + 20 + 4);
		a = gf_bs_read_u8(bs);
		b = gf_bs_read_u8(bs);
		c = gf_bs_read_u8(bs);
		d = gf_bs_read_u8(bs);
		nb_alnum = 0;
		if (isalnum(a)) nb_alnum++;
		if (isalnum(b)) nb_alnum++;
		if (isalnum(c)) nb_alnum++;
		if (isalnum(d)) nb_alnum++;
		if (nb_alnum>2) audioentrybox->qtff_mode = GF_ISOM_AUDIO_QTFF_NONE;
		gf_bs_seek(bs, start);
	}

	e = gf_isom_audio_sample_entry_read((GF_AudioSampleEntryBox*)s, bs);
	if (e) return e;
	pos = gf_bs_get_position(bs);
	size = (u32) s->size;

	//when cookie is set on bs, always convert qtff-style mp4a to isobmff-style
	//since the conversion is done in addBox and we don't have the bitstream there (arg...), flag the box
 	if (gf_bs_get_cookie(bs) & GF_ISOM_BS_COOKIE_QT_CONV) {
 		audioentrybox->qtff_mode |= GF_ISOM_AUDIO_QTFF_CONVERT_FLAG;
 	}

	e = gf_isom_box_array_read(s, bs);
	if (!e) {
		if (s->type==GF_ISOM_BOX_TYPE_ENCA) {
			GF_ProtectionSchemeInfoBox *sinf = (GF_ProtectionSchemeInfoBox *) gf_isom_box_find_child(s->child_boxes, GF_ISOM_BOX_TYPE_SINF);

			if (sinf && sinf->original_format) {
				u32 type = sinf->original_format->data_format;
				switch (type) {
				case GF_ISOM_SUBTYPE_3GP_AMR:
				case GF_ISOM_SUBTYPE_3GP_AMR_WB:
				case GF_ISOM_SUBTYPE_3GP_EVRC:
				case GF_ISOM_SUBTYPE_3GP_QCELP:
				case GF_ISOM_SUBTYPE_3GP_SMV:
					if (audioentrybox->cfg_3gpp) audioentrybox->cfg_3gpp->cfg.type = type;
					break;
				}
			}
		}
		return GF_OK;
	}
	if (size<8) return GF_ISOM_INVALID_FILE;


	/*hack for some weird files (possibly recorded with live.com tools, needs further investigations)*/
	gf_bs_seek(bs, pos);
	data = (char*)gf_malloc(sizeof(char) * size);
	if (!data) return GF_OUT_OF_MEM;

	gf_bs_read_data(bs, data, size);
	for (i=0; i<size-8; i++) {
		if (GF_4CC((u32)data[i+4], (u8)data[i+5], (u8)data[i+6], (u8)data[i+7]) == GF_ISOM_BOX_TYPE_ESDS) {
			GF_BitStream *mybs = gf_bs_new(data + i, size - i, GF_BITSTREAM_READ);
			gf_bs_set_cookie(mybs, GF_ISOM_BS_COOKIE_NO_LOGS);
			// <MASK>
			audioentrybox->esd = NULL;
			e = gf_isom_box_parse((GF_Box **)&audioentrybox->esd, mybs);
			gf_bs_del(mybs);

			if ((e==GF_OK) && audioentrybox->esd && (audioentrybox->esd->type == GF_ISOM_BOX_TYPE_ESDS)) {
				if (!audioentrybox->child_boxes) audioentrybox->child_boxes = gf_list_new();
				gf_list_add(audioentrybox->child_boxes, audioentrybox->esd);
			} else if (audioentrybox->esd) {
				gf_isom_box_del((GF_Box *)audioentrybox->esd);
				audioentrybox->esd = NULL;
			}
			e = GF_OK;
			break;
		}
	}
	gf_free(data);
	return e;
}