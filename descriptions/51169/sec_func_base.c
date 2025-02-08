static void update_errors_warnings(timelib_error_container **last_errors) /* {{{ */
{
	if (DATEG(last_errors)) {
		timelib_error_container_dtor(DATEG(last_errors));
		DATEG(last_errors) = NULL;
	}

	if (last_errors == NULL || (*last_errors) == NULL) {
		return;
	}

	if ((*last_errors)->warning_count || (*last_errors)->error_count) {
		DATEG(last_errors) = *last_errors;
		return;
	}

	timelib_error_container_dtor(*last_errors);
	*last_errors = NULL;
}