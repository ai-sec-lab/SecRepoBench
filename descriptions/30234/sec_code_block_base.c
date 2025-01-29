if (inherit->block != NULL && inherit->block->bi_nodes != NULL) {
		struct cil_tree_node *node;
		struct cil_list_item *item;

		cil_list_for_each(item, inherit->block->bi_nodes) {
			node = item->data;
			if (node->data == inherit) {
				cil_list_remove(inherit->block->bi_nodes, CIL_NODE, node, CIL_FALSE);
				break;
			}
		}
	}

	free(inherit);