static void update_errors_warnings(timelib_error_container **last_errors) /* {{{ */
{
	if (DATEG(last_errors)) {
		timelib_error_container_dtor(DATEG(last_errors));
		DATEG(last_errors) = NULL;
	}

	// <MASK>
	*last_errors = NULL;
}