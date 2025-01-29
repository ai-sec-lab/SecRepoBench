TRACE_SANITIZE (this);
    return_trace (context->check_struct (this) &&
		  typeList.sanitize (context, this,
				     &(this+typeList),
				     data_base));