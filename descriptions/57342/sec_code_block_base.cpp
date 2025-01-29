unsigned avar_axis_count = 0;
  if (has_avar)
  {
    seg_maps = face->table.avar->get_segment_maps ();
    avar_axis_count = face->table.avar->get_axis_count();
  }

  bool axis_not_pinned = false;
  unsigned old_axis_idx = 0, new_axis_idx = 0;
  for (const auto& axis : axes)
  {
    hb_tag_t axis_tag = axis.get_axis_tag ();
    plan->axes_old_index_tag_map.set (old_axis_idx, axis_tag);

    if (!plan->user_axes_location.has (axis_tag))
    {
      axis_not_pinned = true;
      plan->axes_index_map.set (old_axis_idx, new_axis_idx);
      new_axis_idx++;
    }
    else
    {
      int normalized_v = axis.normalize_axis_value (plan->user_axes_location.get (axis_tag));
      if (has_avar && old_axis_idx < avar_axis_count)
      {
        normalized_v = seg_maps->map (normalized_v);
      }
      plan->axes_location.set (axis_tag, normalized_v);
      if (normalized_v != 0)
        plan->pinned_at_default = false;

      plan->normalized_coords[old_axis_idx] = normalized_v;
    }

    old_axis_idx++;

    if (has_avar && old_axis_idx < avar_axis_count)
      seg_maps = &StructAfter<OT::SegmentMaps> (*seg_maps);
  }