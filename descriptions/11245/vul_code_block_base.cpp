TRACE_SANITIZE (this);
    return_trace (true); /* Disabled.  See above. */
    return_trace (c->check_struct (this) &&
		  c->check_range (kernValueZ,
				  kernValueCount * sizeof (FWORD) +
				  glyphCount * 2 +
				  leftClassCount * rightClassCount));