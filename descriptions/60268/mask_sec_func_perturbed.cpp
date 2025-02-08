bool get_points (hb_font_t *font, const accelerator_t &glyf_accelerator,
		   contour_point_vector_t &all_points /* OUT */,
		   contour_point_vector_t *points_with_deltas = nullptr, /* OUT */
		   head_maxp_info_t * head_maxp_info = nullptr, /* OUT */
		   unsigned *composite_contours = nullptr, /* OUT */
		   bool shift_points_hori = true,
		   bool use_my_metrics = true,
		   bool phantom_only = false,
		   hb_array_t<int> coords = hb_array_t<int> (),
		   unsigned int depth = 0,
		   unsigned *edge_count = nullptr) const
  {
    if (unlikely (depth > HB_MAX_NESTING_LEVEL)) return false;
    unsigned stack_edge_count = 0;
    if (!edge_count) edge_count = &stack_edge_count;
    if (unlikely (*edge_count > HB_GLYF_MAX_EDGE_COUNT)) return false;
    (*edge_count)++;

    if (head_maxp_info)
    {
      head_maxp_info->maxComponentDepth = hb_max (head_maxp_info->maxComponentDepth, depth);
    }

    if (!coords)
      coords = hb_array (font->coords, font->num_coords);

    contour_point_vector_t stack_points;
    contour_point_vector_t &points = type == SIMPLE ? all_points : stack_points;
    unsigned old_length = points.length;

    switch (type) {
    case SIMPLE:
      if (depth == 0 && head_maxp_info)
        head_maxp_info->maxContours = hb_max (head_maxp_info->maxContours, (unsigned) header->numberOfContours);
      if (depth > 0 && composite_contours)
        *composite_contours += (unsigned) header->numberOfContours;
      if (unlikely (!SimpleGlyph (*header, bytes).get_contour_points (all_points, phantom_only)))
	return false;
      break;
    case COMPOSITE:
    {
      for (auto &item : get_composite_iterator ())
        if (unlikely (!item.get_points (points))) return false;
      break;
    }
#ifndef HB_NO_VAR_COMPOSITES
    case VAR_COMPOSITE:
    {
      for (auto &item : get_var_composite_iterator ())
        if (unlikely (!item.get_points (points))) return false;
      break;
    }
#endif
    case EMPTY:
      break;
    }

    /* Init phantom points */
    if (unlikely (!points.resize (points.length + PHANTOM_COUNT))) return false;
    hb_array_t<contour_point_t> phantoms = points.as_array ().sub_array (points.length - PHANTOM_COUNT, PHANTOM_COUNT);
    {
      int lsb = 0;
      int h_delta = glyf_accelerator.hmtx->get_leading_bearing_without_var_unscaled (gid, &lsb) ?
		    (int) header->xMin - lsb : 0;
      HB_UNUSED int tsb = 0;
      int v_orig  = (int) header->yMax +
#ifndef HB_NO_VERTICAL
		    ((void) glyf_accelerator.vmtx->get_leading_bearing_without_var_unscaled (gid, &tsb), tsb)
#else
		    0
#endif
		    ;
      unsigned h_adv = glyf_accelerator.hmtx->get_advance_without_var_unscaled (gid);
      unsigned v_adv =
#ifndef HB_NO_VERTICAL
		       glyf_accelerator.vmtx->get_advance_without_var_unscaled (gid)
#else
		       - font->face->get_upem ()
#endif
		       ;
      phantoms[PHANTOM_LEFT].x = h_delta;
      phantoms[PHANTOM_RIGHT].x = (int) h_adv + h_delta;
      phantoms[PHANTOM_TOP].y = v_orig;
      phantoms[PHANTOM_BOTTOM].y = v_orig - (int) v_adv;
    }

#ifndef HB_NO_VAR
    if (coords)
      glyf_accelerator.gvar->apply_deltas_to_points (gid,
						     coords,
						     points.as_array ().sub_array (old_length),
						     phantom_only && type == SIMPLE);
#endif

    // mainly used by CompositeGlyph calculating new X/Y offset value so no need to extend it
    // with child glyphs' points
    if (points_with_deltas != nullptr && depth == 0 && type == COMPOSITE)
    {
      if (unlikely (!points_with_deltas->resize (points.length))) return false;
      *points_with_deltas = points;
    }

    switch (type) {
    case SIMPLE:
      if (depth == 0 && head_maxp_info)
        head_maxp_info->maxPoints = hb_max (head_maxp_info->maxPoints, all_points.length - old_length - 4);
      break;
    case COMPOSITE:
    {
      unsigned int comp_index = 0;
      for (auto &item : get_composite_iterator ())
      {
	unsigned old_count = all_points.length;

	if (unlikely ((!phantom_only || (use_my_metrics && item.is_use_my_metrics ())) &&
		      !glyf_accelerator.glyph_for_gid (item.get_gid ())
				       .get_points (font,
						    glyf_accelerator,
						    all_points,
						    points_with_deltas,
						    head_maxp_info,
						    composite_contours,
						    shift_points_hori,
						    use_my_metrics,
						    phantom_only,
						    coords,
						    depth + 1,
						    edge_count)))
	  return false;

	auto comp_points = all_points.as_array ().sub_array (old_count);

	/* Copy phantom points from component if USE_MY_METRICS flag set */
	if (use_my_metrics && item.is_use_my_metrics ())
	  for (unsigned int index = 0; index < PHANTOM_COUNT; index++)
	    phantoms[index] = comp_points[comp_points.length - PHANTOM_COUNT + index];

	float matrix[4];
	contour_point_t default_trans;
	item.get_transformation (matrix, default_trans);

	// <MASK>

	all_points.resize (all_points.length - PHANTOM_COUNT);

	if (all_points.length > HB_GLYF_MAX_POINTS)
	  return false;

	comp_index++;
      }

      if (head_maxp_info && depth == 0)
      {
        if (composite_contours)
          head_maxp_info->maxCompositeContours = hb_max (head_maxp_info->maxCompositeContours, *composite_contours);
        head_maxp_info->maxCompositePoints = hb_max (head_maxp_info->maxCompositePoints, all_points.length);
        head_maxp_info->maxComponentElements = hb_max (head_maxp_info->maxComponentElements, comp_index);
      }
      all_points.extend (phantoms);
    } break;
#ifndef HB_NO_VAR_COMPOSITES
    case VAR_COMPOSITE:
    {
      hb_array_t<contour_point_t> points_left = points.as_array ();
      for (auto &item : get_var_composite_iterator ())
      {
	unsigned item_num_points = item.get_num_points ();
	hb_array_t<contour_point_t> record_points = points_left.sub_array (0, item_num_points);
	assert (record_points.length == item_num_points);

	auto component_coords = coords;
	/* Copying coords is expensive; so we have put an arbitrary
	 * limit on the max number of coords for now. */
	if (item.is_reset_unspecified_axes () ||
	    coords.length > HB_GLYF_VAR_COMPOSITE_MAX_AXES)
	  component_coords = hb_array<int> ();

	coord_setter_t coord_setter (component_coords);
	item.set_variations (coord_setter, record_points);

	unsigned old_count = all_points.length;

	if (unlikely ((!phantom_only || (use_my_metrics && item.is_use_my_metrics ())) &&
		      !glyf_accelerator.glyph_for_gid (item.get_gid ())
				       .get_points (font,
						    glyf_accelerator,
						    all_points,
						    points_with_deltas,
						    head_maxp_info,
						    nullptr,
						    shift_points_hori,
						    use_my_metrics,
						    phantom_only,
						    coord_setter.get_coords (),
						    depth + 1,
						    edge_count)))
	  return false;

	auto comp_points = all_points.as_array ().sub_array (old_count);

	/* Apply component transformation */
	if (comp_points) // Empty in case of phantom_only
	  item.transform_points (record_points, comp_points);

	/* Copy phantom points from component if USE_MY_METRICS flag set */
	if (use_my_metrics && item.is_use_my_metrics ())
	  for (unsigned int index = 0; index < PHANTOM_COUNT; index++)
	    phantoms[index] = comp_points[comp_points.length - PHANTOM_COUNT + index];

	all_points.resize (all_points.length - PHANTOM_COUNT);

	if (all_points.length > HB_GLYF_MAX_POINTS)
	  return false;

	points_left += item_num_points;
      }
      all_points.extend (phantoms);
    } break;
#endif
    case EMPTY:
      all_points.extend (phantoms);
      break;
    }

    if (depth == 0 && shift_points_hori) /* Apply at top level */
    {
      /* Undocumented rasterizer behavior:
       * Shift points horizontally by the updated left side bearing
       */
      int v = -phantoms[PHANTOM_LEFT].x;
      if (v)
        for (auto &point : all_points)
	  point.x += v;
    }

    return !all_points.in_error ();
  }