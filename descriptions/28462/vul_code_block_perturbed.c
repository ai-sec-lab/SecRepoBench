for (curr = blockinheritnode; curr != terminating_node; curr = curr->parent) {
		if (curr->flavor == CIL_BLOCK) {
			cil_list_prepend(trace, CIL_NODE, curr);
		} else {
			if (curr != blockinheritnode) {
				cil_list_prepend(trace, CIL_NODE, NODE(((struct cil_blockinherit *)curr->data)->block));
			}
			cil_list_prepend(trace, CIL_NODE, curr);
		}
	}
	cil_list_prepend(trace, CIL_NODE, terminating_node);

	cil_list_for_each(item, trace) {
		curr = item->data;
		if (curr->flavor == CIL_BLOCK) {
			cil_tree_log(curr, CIL_ERR, "block %s", DATUM(curr->data)->name);
		} else {
			cil_tree_log(curr, CIL_ERR, "blockinherit %s", ((struct cil_blockinherit *)curr->data)->block_str);
		}
	}