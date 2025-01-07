
    hb_vector_t<unsigned> shared_indices;
    TupleVariationData::tuple_iterator_t iterator;
    unsigned var_data_length = tupleVariationData.get_size (axis_count);
    hb_bytes_t var_data_bytes = hb_bytes_t (reinterpret_cast<const char*> (get_tuple_var_data ()), var_data_length);
    if (!TupleVariationData::get_tuple_iterator (var_data_bytes, axis_count, this,
                                                 shared_indices, &iterator))
      return false;
    
    return tupleVariationData.decompile_tuple_variations (point_count, is_gvar, iterator,
                                                          axes_old_index_tag_map,
                                                          shared_indices,
                                                          hb_array<const F2DOT14> (),
                                                          tuple_variations);
  