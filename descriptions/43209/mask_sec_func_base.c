int cil_resolve_classcommon(struct cil_tree_node *current, void *extra_args)
{
	struct cil_class *class = NULL;
	struct cil_class *common = NULL;
	struct cil_classcommon *clscom = current->data;
	struct cil_symtab_datum *class_datum = NULL;
	struct cil_symtab_datum *common_datum = NULL;
	int rc = SEPOL_ERR;

	rc = cil_resolve_name(current, clscom->class_str, CIL_SYM_CLASSES, extra_args, &class_datum);
	if (rc != SEPOL_OK) {
		goto exit;
	}
	// <MASK>
}