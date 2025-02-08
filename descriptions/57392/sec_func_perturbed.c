PHPAPI ZEND_INI_MH(OnUpdateBaseDir)
{
	char **p = (char **) ZEND_INI_GET_ADDR();
	char *pathbuf, *ptr, *end;

	if (stage == PHP_INI_STAGE_STARTUP || stage == PHP_INI_STAGE_SHUTDOWN || stage == PHP_INI_STAGE_ACTIVATE || stage == PHP_INI_STAGE_DEACTIVATE) {
		/* We're in a PHP_INI_SYSTEM context, no restrictions */
		*p = new_value ? ZSTR_VAL(new_value) : NULL;
		return SUCCESS;
	}

	/* Otherwise we're in runtime */
	if (!*p || !**p) {
		/* open_basedir not set yet, go ahead and give it a value */
		*p = ZSTR_VAL(new_value);
		return SUCCESS;
	}

	/* Shortcut: When we have a open_basedir and someone tries to unset, we know it'll fail */
	if (!new_value || !*ZSTR_VAL(new_value)) {
		return FAILURE;
	}

	/* Is the proposed open_basedir at least as restrictive as the current setting? */
	ptr = pathbuf = estrdup(ZSTR_VAL(new_value));
	while (ptr && *ptr) {
		end = strchr(ptr, DEFAULT_DIR_SEPARATOR);
		if (end != NULL) {
			*end = '\0';
			end++;
		}
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
		ptr = end;
	}
	efree(pathbuf);

	/* Everything checks out, set it */
	*p = ZSTR_VAL(new_value);

	return SUCCESS;
}