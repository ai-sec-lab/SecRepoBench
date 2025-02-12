
    hb_hashmap_t<hb_codepoint_t, hb_bytes_t> new_gid_var_data_map;
    auto it = hb_iter (plan->new_to_old_gid_list);
    if (it->first == 0 && !(plan->flags & HB_SUBSET_FLAGS_NOTDEF_OUTLINE))
    {
      new_gid_var_data_map.set (0, hb_bytes_t ());
      it++;
    }

    for (auto &_ : it)
    {
      hb_codepoint_t new_gid = _.first;
      hb_codepoint_t old_gid = _.second;
      hb_bytes_t var_data_bytes = get_glyph_var_data_bytes (old_gid);
      new_gid_var_data_map.set (new_gid, var_data_bytes);
    }

    if (new_gid_var_data_map.in_error ()) return false;

    hb_array_t<const F2DOT14> shared_tuples = (this+sharedTuples).as_array ((unsigned) sharedTupleCount * (unsigned) axisCount);
    return glyph_vars.create_from_glyphs_var_data (axisCount, shared_tuples, plan, new_gid_var_data_map);