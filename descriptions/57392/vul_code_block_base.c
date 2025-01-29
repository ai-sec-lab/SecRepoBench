if (ptr[0] == '.' && ptr[1] == '.' && (ptr[2] == '\0' || IS_SLASH(ptr[2]))) {
			/* Don't allow paths with a leading .. path component to be set at runtime */
			efree(pathbuf);
			return FAILURE;
		}
		if (php_check_open_basedir_ex(ptr, 0) != 0) {
			/* At least one portion of this open_basedir is less restrictive than the prior one, FAIL */
			efree(pathbuf);
			return FAILURE;
		}