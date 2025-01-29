if (ptr->esd) {

				gf_list_del_item(ptr->child_boxes, (GF_Box *)ptr->esd);

				for (u32 i=0; i<gf_list_count(ptr->child_boxes); i++) {
					GF_Box *inner_box = (GF_Box *)gf_list_get(ptr->child_boxes, i);
					if (inner_box->child_boxes) {
						gf_list_del_item(inner_box->child_boxes, (GF_Box *)ptr->esd);
					}
				}

				gf_isom_box_del((GF_Box *)ptr->esd);
			}
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