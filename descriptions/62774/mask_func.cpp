bool decompile_tuple_variations (unsigned axis_count,
                                   unsigned point_count,
                                   hb_blob_t *blob,
                                   bool is_gvar,
                                   const hb_map_t *axes_old_index_tag_map,
                                   TupleVariationData::tuple_variations_t& tuple_variations /* OUT */) const
  {
    // <MASK>
    
    return tupleVariationData.decompile_tuple_variations (point_count, is_gvar, iterator,
                                                          axes_old_index_tag_map,
                                                          shared_indices,
                                                          hb_array<const F2DOT14> (),
                                                          tuple_variations);
  }