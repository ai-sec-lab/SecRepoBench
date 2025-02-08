TRACE_SANITIZE (this);
    const void *type_base = &(this+typeList);
    return_trace (context->check_struct (this) &&
		  typeList.sanitize (context, this,
				     type_base,
				     data_base));