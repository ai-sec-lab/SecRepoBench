int cil_resolve_classpermissionset(struct cil_tree_node *current, struct cil_classpermissionset *cps, struct cil_db *db)
{
	int resultcode = SEPOL_ERR;
	struct cil_list_item *curr;
	struct cil_symtab_datum *datum;
	struct cil_classpermission *cp;

	resultcode = cil_resolve_name(current, cps->set_str, CIL_SYM_CLASSPERMSETS, db, &datum);
	if (resultcode != SEPOL_OK) {
		goto exit;
	}

	// <MASK>
}