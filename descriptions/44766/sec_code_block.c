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