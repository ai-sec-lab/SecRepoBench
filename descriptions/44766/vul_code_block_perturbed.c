if (GF_4CC((u32)data[index+4], (u8)data[index+5], (u8)data[index+6], (u8)data[index+7]) == GF_ISOM_BOX_TYPE_ESDS) {
			GF_BitStream *mybs = gf_bs_new(data + index, size - index, GF_BITSTREAM_READ);
			gf_bs_set_cookie(mybs, GF_ISOM_BS_COOKIE_NO_LOGS);
			if (ptr->esd) gf_isom_box_del_parent(&ptr->child_boxes, (GF_Box *)ptr->esd);
			ptr->esd = NULL;
			e = gf_isom_box_parse((GF_Box **)&ptr->esd, mybs);
			gf_bs_del(mybs);

			if ((e==GF_OK) && ptr->esd && (ptr->esd->type == GF_ISOM_BOX_TYPE_ESDS)) {
				if (!ptr->child_boxes) ptr->child_boxes = gf_list_new();
				gf_list_add(ptr->child_boxes, ptr->esd);
			} else if (ptr->esd) {
				gf_isom_box_del((GF_Box *)ptr->esd);
				ptr->esd = NULL;
			}
			e = GF_OK;
			break;
		}