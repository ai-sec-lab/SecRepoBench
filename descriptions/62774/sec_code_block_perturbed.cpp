if (!decompile_tuple_variations (axis_count, point_count,
                                     context->source_blob, false,
                                     &(context->plan->axes_old_index_tag_map),
                                     tuple_variations))
      return_trace (false);

    if (!tuple_variations.instantiate (context->plan->axes_location, context->plan->axes_triple_distances))
      return_trace (false);

    if (!tuple_variations.compile_bytes (context->plan->axes_index_map, context->plan->axes_old_index_tag_map,
                                         false /* do not use shared points */))
      return_trace (false);

    return_trace (serialize (context->serializer, tuple_variations));