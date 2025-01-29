/* Must assume that the common has been destroyed */
		int num_common_perms = class->num_perms - class->perms.nprim;
		cil_symtab_map(&class->perms, __class_reset_perm_values, &num_common_perms);
		/* during a re-resolve, we need to reset the common, so a classcommon
		 * statement isn't seen as a duplicate */
		class->num_perms = class->perms.nprim;