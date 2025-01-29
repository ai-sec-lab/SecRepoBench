
	switch (str_expr->flavor) {
	case CIL_BOOL:
		sym_index = CIL_SYM_BOOLS;
		break;
	case CIL_TUNABLE:
		sym_index = CIL_SYM_TUNABLES;
		break;
	case CIL_TYPE:
		sym_index = CIL_SYM_TYPES;
		break;
	case CIL_ROLE:
		sym_index = CIL_SYM_ROLES;
		break;
	case CIL_USER:
		sym_index = CIL_SYM_USERS;
		break;
	case CIL_CAT:
		sym_index = CIL_SYM_CATS;
		break;
	default:
		break;
	}

	cil_list_init(datum_expr, str_expr->flavor);

	cil_list_for_each(curr, str_expr) {
		switch (curr->flavor) {
		case CIL_STRING:
			rc = cil_resolve_name(parent, curr->data, sym_index, extra_args, &res_datum);
			if (rc != SEPOL_OK) {
				goto exit;
			}
			if (sym_index == CIL_SYM_CATS && NODE(res_datum)->flavor == CIL_CATSET) {
				struct cil_catset *catset = (struct cil_catset *)res_datum;
				if (!catset->cats->datum_expr) {
					rc = cil_resolve_expr(expr_type, catset->cats->str_expr, &catset->cats->datum_expr, parent, extra_args);
					if (rc != SEPOL_OK) {
						goto exit;
					}
				}
				cil_copy_list(catset->cats->datum_expr, &datum_sub_expr);
				cil_list_append(*datum_expr, CIL_LIST, datum_sub_expr);
			} else {
				if (sym_index == CIL_SYM_TYPES && (expr_type == CIL_CONSTRAIN || expr_type == CIL_VALIDATETRANS)) {
					cil_type_used(res_datum, CIL_ATTR_CONSTRAINT);
				}
				cil_list_append(*datum_expr, CIL_DATUM, res_datum);
			}
			break;
		case CIL_LIST: {
			rc = cil_resolve_expr(expr_type, curr->data, &datum_sub_expr, parent, extra_args);
			if (rc != SEPOL_OK) {
				goto exit;
			}
			cil_list_append(*datum_expr, CIL_LIST, datum_sub_expr);
			break;
		}
		default:
			cil_list_append(*datum_expr, curr->flavor, curr->data);
			break;
		}				
	}