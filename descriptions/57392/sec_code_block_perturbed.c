/* Don't allow paths with a parent dir component (..) to be set at runtime */
		char *substr_pos = ptr;
		while (true) {
			// Check if we have a .. path component
			if (substr_pos[0] == '.'
			 && substr_pos[1] == '.'
			 && (substr_pos[2] == '\0' || IS_SLASH(substr_pos[2]))) {
				efree(pathbuf);
				return FAILURE;
			}
			// Skip to the next path component
			while (true) {
				substr_pos++;
				if (*substr_pos == '\0' || *substr_pos == DEFAULT_DIR_SEPARATOR) {
					goto no_parent_dir_component;
				} else if (IS_SLASH(*substr_pos)) {
					// Also skip the slash
					substr_pos++;
					break;
				}
			}
		}
no_parent_dir_component:
		if (php_check_open_basedir_ex(ptr, 0) != 0) {
			/* At least one portion of this open_basedir is less restrictive than the prior one, FAIL */
			efree(pathbuf);
			return FAILURE;
		}