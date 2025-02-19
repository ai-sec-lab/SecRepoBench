if (audioentrybox->esd) {

				gf_list_del_item(audioentrybox->child_boxes, (GF_Box *)audioentrybox->esd);

				for (u32 i=0; i<gf_list_count(audioentrybox->child_boxes); i++) {
					GF_Box *inner_box = (GF_Box *)gf_list_get(audioentrybox->child_boxes, i);
					if (inner_box->child_boxes) {
						gf_list_del_item(inner_box->child_boxes, (GF_Box *)audioentrybox->esd);
					}
				}

				gf_isom_box_del((GF_Box *)audioentrybox->esd);
			}