if (validate_cond_expr(handle, cond->expr, &flavors[SYM_BOOLS]))
			goto bad;
		if (validate_cond_av_list(handle, cond->true_list, flavors))
			goto bad;
		if (validate_cond_av_list(handle, cond->false_list, flavors))
			goto bad;
		if (validate_avrules(handle, cond->avtrue_list, 1, flavors))
			goto bad;
		if (validate_avrules(handle, cond->avfalse_list, 1, flavors))
			goto bad;
		if (validate_bool_id_array(handle, cond->bool_ids, cond->nbools, &flavors[SYM_BOOLS]))
			goto bad;