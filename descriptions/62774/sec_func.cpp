bool decompile_tuple_variations (unsigned axis_count,
                                   unsigned point_count,
                                   hb_blob_t *blob,
                                   bool is_gvar,
                                   const hb_map_t *axes_old_index_tag_map,
                                   TupleVariationData::tuple_variations_t& tuple_variations /* OUT */) const
  {
    hb_vector_t<unsigned> shared_indices;
    TupleVariationData::tuple_iterator_t iterator;
    hb_bytes_t var_data_bytes = blob->as_bytes ().sub_array (4);
    if (!TupleVariationData::get_tuple_iterator (var_data_bytes, axis_count, this,
                                                 shared_indices, &iterator))
      return false;
    
    return tupleVariationData.decompile_tuple_variations (point_count, is_gvar, iterator,
                                                          axes_old_index_tag_map,
                                                          shared_indices,
                                                          hb_array<const F2DOT14> (),
                                                          tuple_variations);
  }