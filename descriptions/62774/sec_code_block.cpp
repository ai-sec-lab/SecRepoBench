hb_vector_t<unsigned> shared_indices;
    TupleVariationData::tuple_iterator_t iterator;
    hb_bytes_t var_data_bytes = blob->as_bytes ().sub_array (4);
    if (!TupleVariationData::get_tuple_iterator (var_data_bytes, axis_count, this,
                                                 shared_indices, &iterator))
      return false;