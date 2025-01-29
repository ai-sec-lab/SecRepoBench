for (obj = p15card->obj_list; obj != NULL; obj = obj->next) {
		if (obj->df != df)
			continue;
		r = func(ctx, obj, &tmp, &tmpsize);
		if (r) {
			free(tmp);
			free(buf);
			return r;
		}
		if (!tmpsize)
			continue;
		p = (u8 *) realloc(buf, bufsize + tmpsize);
		if (!p) {
			free(tmp);
			free(buf);
			return SC_ERROR_OUT_OF_MEMORY;
		}
		buf = p;
		memcpy(buf + bufsize, tmp, tmpsize);
		free(tmp);
		tmp = NULL;
		bufsize += tmpsize;
	}
	*buf_out = buf;
	*bufsize_out = bufsize;

	return 0;