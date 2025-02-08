int cil_resolve_ast(struct cil_db *db, struct cil_tree_node *current)
{
	int rc = SEPOL_ERR;
	struct cil_args_resolve extra_args;
	enum cil_pass pass = CILPASSTYPEINFO;
	uint32_t changed = 0;

	if (db == NULL || current == NULL) {
		return rc;
	}

	extra_args.db = db;
	extra_args.pass = pass;
	extra_args.changed = &changed;
	extra_args.block = NULL;
	extra_args.macro = NULL;
	extra_args.optional = NULL;
	extra_args.boolif= NULL;
	extra_args.sidorder_lists = NULL;
	extra_args.classorder_lists = NULL;
	extra_args.unordered_classorder_lists = NULL;
	extra_args.catorder_lists = NULL;
	extra_args.sensitivityorder_lists = NULL;
	extra_args.in_list = NULL;

	cil_list_init(&extra_args.disabled_optionals, CIL_NODE);
	cil_list_init(&extra_args.sidorder_lists, CIL_LIST_ITEM);
	cil_list_init(&extra_args.classorder_lists, CIL_LIST_ITEM);
	cil_list_init(&extra_args.unordered_classorder_lists, CIL_LIST_ITEM);
	cil_list_init(&extra_args.catorder_lists, CIL_LIST_ITEM);
	cil_list_init(&extra_args.sensitivityorder_lists, CIL_LIST_ITEM);
	cil_list_init(&extra_args.in_list, CIL_IN);
	for (pass = CILPASSTYPEINFO; pass < CIL_PASS_NUM; pass++) {
		// <MASK>
	}

	rc = __cil_verify_initsids(db->sidorder);
	if (rc != SEPOL_OK) {
		goto exit;
	}

	rc = SEPOL_OK;
exit:
	__cil_ordered_lists_destroy(&extra_args.sidorder_lists);
	__cil_ordered_lists_destroy(&extra_args.classorder_lists);
	__cil_ordered_lists_destroy(&extra_args.catorder_lists);
	__cil_ordered_lists_destroy(&extra_args.sensitivityorder_lists);
	__cil_ordered_lists_destroy(&extra_args.unordered_classorder_lists);
	cil_list_destroy(&extra_args.disabled_optionals, CIL_FALSE);
	cil_list_destroy(&extra_args.in_list, CIL_FALSE);

	return rc;
}