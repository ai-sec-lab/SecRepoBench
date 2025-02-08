TRACE_SANITIZE (this);
    const void *type_base = &(this+typeList);
    return_trace (c->check_struct (this) &&
		  typeList.sanitize (c, this,
				     type_base,
				     data_base));