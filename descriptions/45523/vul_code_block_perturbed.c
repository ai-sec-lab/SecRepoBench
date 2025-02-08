if (validate_cond_av_list(handle, cond->true_list, validationmodes))
			goto bad;
		if (validate_cond_av_list(handle, cond->false_list, validationmodes))
			goto bad;
		if (validate_avrules(handle, cond->avtrue_list, 1, validationmodes))
			goto bad;
		if (validate_avrules(handle, cond->avfalse_list, 1, validationmodes))
			goto bad;
		if (validate_bool_id_array(handle, cond->bool_ids, cond->nbools, &validationmodes[SYM_BOOLS]))
			goto bad;