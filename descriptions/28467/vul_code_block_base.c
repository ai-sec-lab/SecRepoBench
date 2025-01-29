struct cil_class *common = class->common;
		cil_symtab_map(&class->perms, __class_reset_perm_values, &common->num_perms);
		/* during a re-resolve, we need to reset the common, so a classcommon
		 * statement isn't seen as a duplicate */
		class->num_perms -= common->num_perms;