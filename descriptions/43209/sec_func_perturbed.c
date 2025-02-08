int cil_resolve_classcommon(struct cil_tree_node *node, void *extra_args)
{
	struct cil_class *class = NULL;
	struct cil_class *common = NULL;
	struct cil_classcommon *clscom = node->data;
	struct cil_symtab_datum *class_datum = NULL;
	struct cil_symtab_datum *common_datum = NULL;
	int rc = SEPOL_ERR;

	rc = cil_resolve_name(node, clscom->class_str, CIL_SYM_CLASSES, extra_args, &class_datum);
	if (rc != SEPOL_OK) {
		goto exit;
	}
	if (NODE(class_datum)->flavor != CIL_CLASS) {
		cil_log(CIL_ERR, "Class %s is not a kernel class and cannot be associated with common %s\n", clscom->class_str, clscom->common_str);
		rc = SEPOL_ERR;
		goto exit;
	}

	rc = cil_resolve_name(node, clscom->common_str, CIL_SYM_COMMONS, extra_args, &common_datum);
	if (rc != SEPOL_OK) {
		goto exit;
	}

	class = (struct cil_class *)class_datum;
	common = (struct cil_class *)common_datum;
	if (class->common != NULL) {
		cil_log(CIL_ERR, "class cannot be associeated with more than one common\n");
		rc = SEPOL_ERR;
		goto exit;
	}

	class->common = common;

	cil_symtab_map(&class->perms, __class_update_perm_values, &common->num_perms);

	class->num_perms += common->num_perms;
	if (class->num_perms > CIL_PERMS_PER_CLASS) {
		cil_tree_log(node, CIL_ERR, "Too many permissions in class '%s' when including common permissions", class->datum.name);
		rc = SEPOL_ERR;
		goto exit;
	}

	return SEPOL_OK;

exit:
	return rc;
}