int cil_resolve_expr(enum cil_flavor expr_type, struct cil_list *str_expr, struct cil_list **datum_expr, struct cil_tree_node *parent, void *extra_args)
{
	int rc = SEPOL_ERR;
	struct cil_list_item *curr;
	struct cil_symtab_datum *res_datum = NULL;
	enum cil_sym_index sym_index =  CIL_SYM_UNKNOWN;
	struct cil_list *datum_sub_expr;
	// <MASK>
	return SEPOL_OK;

exit:
	cil_list_destroy(datum_expr, CIL_FALSE);
	return rc;
}