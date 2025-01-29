(index < var_data->tupleVarCount.get_count ()) &&
	     var_data_bytes.check_range (current_tuple, TupleVarHeader::min_size) &&
	     var_data_bytes.check_range (current_tuple, current_tuple->get_data_size ()) &&
	     current_tuple->get_size (axis_count)