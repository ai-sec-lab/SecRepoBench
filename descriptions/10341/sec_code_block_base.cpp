TRACE_SANITIZE (this);
    return_trace (c->check_struct (this) &&
		  typeList.sanitize (c, this,
				     &(this+typeList),
				     data_base));