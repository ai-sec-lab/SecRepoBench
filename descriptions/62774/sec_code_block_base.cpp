if (!decompile_tuple_variations (axis_count, point_count,
                                     c->source_blob, false,
                                     &(c->plan->axes_old_index_tag_map),
                                     tuple_variations))
      return_trace (false);

    if (!tuple_variations.instantiate (c->plan->axes_location, c->plan->axes_triple_distances))
      return_trace (false);

    if (!tuple_variations.compile_bytes (c->plan->axes_index_map, c->plan->axes_old_index_tag_map,
                                         false /* do not use shared points */))
      return_trace (false);

    return_trace (serialize (c->serializer, tuple_variations));