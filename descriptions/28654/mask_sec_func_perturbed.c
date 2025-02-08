static int __cil_fill_constraint_leaf_expr(struct cil_tree_node *current, enum cil_flavor expr_flavor, enum cil_flavor op, struct cil_list **leaf_expr)
{
	int rc = SEPOL_ERR;
	enum cil_flavor leafexprkind = CIL_NONE;
	enum cil_flavor l_flavor = CIL_NONE;
	enum cil_flavor r_flavor = CIL_NONE;

	l_flavor = __cil_get_constraint_operand_flavor(current->next->data);
	r_flavor = __cil_get_constraint_operand_flavor(current->next->next->data);

	switch (l_flavor) {
	case CIL_CONS_U1:
	case CIL_CONS_U2:
	case CIL_CONS_U3:
		leafexprkind = CIL_USER;
		break;
	case CIL_CONS_R1:
	case CIL_CONS_R2:
	case CIL_CONS_R3:
		leafexprkind = CIL_ROLE;
		break;
	case CIL_CONS_T1:
	case CIL_CONS_T2:
	case CIL_CONS_T3:
		leafexprkind = CIL_TYPE;
		break;
	case CIL_CONS_L1:
	case CIL_CONS_L2:
	case CIL_CONS_H1:
	case CIL_CONS_H2:
		leafexprkind = CIL_LEVEL;
		break;
	default:
		cil_log(CIL_ERR, "Invalid left operand (%s)\n", (char*)current->next->data);
		goto exit;
	}

	rc = cil_verify_constraint_leaf_expr_syntax(l_flavor, r_flavor, op, expr_flavor);
	if (rc != SEPOL_OK) {
		goto exit;
	}

	cil_list_init(leaf_expr, leafexprkind);

	cil_list_append(*leaf_expr, CIL_OP, (void *)op);

	cil_list_append(*leaf_expr, CIL_CONS_OPERAND, (void *)l_flavor);

	if (r_flavor == CIL_STRING) {
		cil_list_append(*leaf_expr, CIL_STRING, current->next->next->data);
	} else if (r_flavor == CIL_LIST) {
		struct cil_list *sub_list;
		// <MASK>
	} else {
		cil_list_append(*leaf_expr, CIL_CONS_OPERAND, (void *)r_flavor);
	}

	return SEPOL_OK;

exit:

	return SEPOL_ERR;
}