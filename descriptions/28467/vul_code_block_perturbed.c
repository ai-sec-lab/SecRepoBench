struct cil_class *common = cls->common;
		cil_symtab_map(&cls->perms, __class_reset_perm_values, &common->num_perms);
		/* during a re-resolve, we need to reset the common, so a classcommon
		 * statement isn't seen as a duplicate */
		cls->num_perms -= common->num_perms;