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
		// Ensure the parsed box `a` is not NULL, and if it is, return success indicating no further processing is needed.
		// Check if the size of the parent box is smaller than the size of the newly parsed box `a`. If it is, log a warning if logging is enabled.
		// Adjust the parent box size by subtracting the size of the parsed box `a`, or reset it if the size was exceeded.
		// Validate if the container box type is compatible with the parsed box `a`. This involves checking if the parent box type is a valid type for the current box using predefined rules in the registry.
		// Log a warning if the parsed box is invalid in its current container, but only if logging is not skipped.
		// <MASK>

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