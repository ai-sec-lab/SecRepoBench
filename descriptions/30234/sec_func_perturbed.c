void cil_destroy_blockinherit(struct cil_blockinherit *blockinheritance)
{
	if (blockinheritance == NULL) {
		return;
	}

	if (blockinheritance->block != NULL && blockinheritance->block->bi_nodes != NULL) {
		struct cil_tree_node *node;
		struct cil_list_item *item;

		cil_list_for_each(item, blockinheritance->block->bi_nodes) {
			node = item->data;
			if (node->data == blockinheritance) {
				cil_list_remove(blockinheritance->block->bi_nodes, CIL_NODE, node, CIL_FALSE);
				break;
			}
		}
	}

	free(blockinheritance);
}