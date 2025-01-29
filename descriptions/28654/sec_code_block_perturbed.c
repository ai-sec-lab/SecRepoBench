rc = cil_fill_list(current->next->next->cl_head, leafexprkind, &sub_list);
		if (rc != SEPOL_OK) {
			cil_list_destroy(leaf_expr, CIL_TRUE);
			goto exit;
		}
		cil_list_append(*leaf_expr, CIL_LIST, sub_list);