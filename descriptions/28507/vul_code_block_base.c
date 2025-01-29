{
		struct cil_list *sub_list;
		cil_fill_list(current->next->next->cl_head, leaf_expr_flavor, &sub_list);
		cil_list_append(*leaf_expr, CIL_LIST, &sub_list);
	} else {
		cil_list_append(*leaf_expr, CIL_CONS_OPERAND, (void *)r_flavor);
	}