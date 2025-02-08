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

	if (!datum->fqn) {
		cil_tree_log(current, CIL_ERR, "Anonymous classpermission used in a classpermissionset");
		resultcode = SEPOL_ERR;
		goto exit;
	}

	resultcode = cil_resolve_classperms_list(current, cps->classperms, db);
	if (resultcode != SEPOL_OK) {
		goto exit;
	}

	cp = (struct cil_classpermission *)datum;
	cps->set = cp;

	if (cp->classperms == NULL) {
		cil_list_init(&cp->classperms, CIL_CLASSPERMS);
	}

	cil_list_for_each(curr, cps->classperms) {
		cil_list_append(cp->classperms, curr->flavor, curr->data);
	}

	return SEPOL_OK;

exit:
	return resultcode;
}