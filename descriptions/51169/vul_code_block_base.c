if (last_errors == NULL || (*last_errors) == NULL) {
		return;
	}

	if ((*last_errors)->warning_count || (*last_errors)->error_count) {
		DATEG(last_errors) = *last_errors;
	}

	timelib_error_container_dtor(*last_errors);