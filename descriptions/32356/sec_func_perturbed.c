GF_Err gf_isom_box_array_read_ex(GF_Box *container, GF_BitStream *bs, u32 parent_type)
{
	GF_Err e;
	GF_Box *a = NULL;
	Bool skip_logs = (gf_bs_get_cookie(bs) & GF_ISOM_BS_COOKIE_NO_LOGS ) ? GF_TRUE : GF_FALSE;

	//we may have terminators in some QT files (4 bytes set to 0 ...)
	while (container->size>=8) {
		e = gf_isom_box_parse_ex(&a, bs, parent_type, GF_FALSE);
		if (e) {
			if (a) gf_isom_box_del(a);
			return e;
		}
		//sub box parsing aborted with no error
		if (!a) return GF_OK;

		if (container->size < a->size) {
			if (!skip_logs) {
				GF_LOG(GF_LOG_WARNING, GF_LOG_CONTAINER, ("[iso file] Box \"%s\" is larger than container box\n", gf_4cc_to_str(a->type)));
			}
			if (!gf_opts_get_bool("core", "no-check")) {
				gf_isom_box_del(a);
				return GF_ISOM_INVALID_FILE;
			}
			container->size = 0;
		} else {
			container->size -= a->size;
		}

		//check container validity
		if (strlen(a->registry->parents_4cc)) {
			Bool parent_OK = GF_FALSE;
			const char *parent_code = gf_4cc_to_str(container->type);
			if (container->type == GF_ISOM_BOX_TYPE_UNKNOWN)
				parent_code = gf_4cc_to_str( ((GF_UnknownBox*)container)->original_4cc );
			if (strstr(a->registry->parents_4cc, parent_code) != NULL) {
				parent_OK = GF_TRUE;
			} else if (!strcmp(a->registry->parents_4cc, "*") || strstr(a->registry->parents_4cc, "* ") || strstr(a->registry->parents_4cc, " *")) {
				parent_OK = GF_TRUE;
			} else {
				//parent must be a sample entry
				if (strstr(a->registry->parents_4cc, "sample_entry") !=	NULL) {
					//parent is in an stsd
					if (strstr(container->registry->parents_4cc, "stsd") != NULL) {
						if (strstr(a->registry->parents_4cc, "video_sample_entry") !=	NULL) {
							if (((GF_SampleEntryBox*)container)->internal_type==GF_ISOM_SAMPLE_ENTRY_VIDEO) {
								parent_OK = GF_TRUE;
							}
						} else {
							parent_OK = GF_TRUE;
						}
					}
				}
				//other types are sample formats, eg a 3GPP text sample, RTP hint sample or VTT cue. Not handled at this level
				else if (a->type==GF_ISOM_BOX_TYPE_UNKNOWN) parent_OK = GF_TRUE;
				else if (a->type==GF_ISOM_BOX_TYPE_UUID) parent_OK = GF_TRUE;
			}
			if (! parent_OK && !skip_logs) {
				GF_LOG(GF_LOG_WARNING, GF_LOG_CONTAINER, ("[iso file] Box \"%s\" is invalid in container %s\n", gf_4cc_to_str(a->type), parent_code));
			}
		}

		//always register boxes
		if (!container->child_boxes) {
			container->child_boxes = gf_list_new();
			if (!container->child_boxes) return GF_OUT_OF_MEM;
		}
		e = gf_list_add(container->child_boxes, a);
		if (e) return e;

		if (container->registry->add_rem_fn) {
			e = container->registry->add_rem_fn(container, a, GF_FALSE);
			if (e) {
				if (e == GF_ISOM_INVALID_MEDIA) return GF_OK;
				//if the box is no longer present, consider it destroyed
				if (gf_list_find(container->child_boxes, a) >=0) {
					gf_isom_box_del_parent(&container->child_boxes, a);
				}
				return e;
			}
		}
	}
	return GF_OK;
}