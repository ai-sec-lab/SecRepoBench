static struct cil_name * __cil_insert_name(struct cil_db *db, hashtab_key_t key, struct cil_tree_node *ast_node)
{
	/* Currently only used for typetransition file names.
	   But could be used for any string that is passed as a parameter.
	*/
	struct cil_tree_node *parent = ast_node->parent;
	struct cil_macro *macro = NULL;
	struct cil_name *name;
	symtab_t *symtab;
	enum cil_sym_index sym_index;
	struct cil_symtab_datum *datum = NULL;

	if (parent->flavor == CIL_CALL) {
		struct cil_call *call = parent->data;
		macro = call->macro;	
	} else if (parent->flavor == CIL_MACRO) {
		macro = parent->data;
	}
	if (macro != NULL && macro->params != NULL) {
		struct cil_list_item *item;
		cil_list_for_each(item, macro->params) {
			// <MASK>
		}
	}

	cil_flavor_to_symtab_index(CIL_NAME, &sym_index);
	symtab = &((struct cil_root *)db->ast->root->data)->symtab[sym_index];

	cil_symtab_get_datum(symtab, key, &datum);
	if (datum != NULL) {
		return (struct cil_name *)datum;
	}

	cil_name_init(&name);
	cil_symtab_insert(symtab, key, (struct cil_symtab_datum *)name, ast_node);
	cil_list_append(db->names, CIL_NAME, name);

	return name;
}