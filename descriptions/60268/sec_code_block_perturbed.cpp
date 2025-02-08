/* Apply component transformation & translation (with deltas applied) */
	item.transform_points (comp_points, matrix, points[comp_index]);

	if (item.is_anchored () && !phantom_only)
	{
	  unsigned int p1, p2;
	  item.get_anchor_points (p1, p2);
	  if (likely (p1 < all_points.length && p2 < comp_points.length))
	  {
	    contour_point_t delta;
	    delta.init (all_points[p1].x - comp_points[p2].x,
			all_points[p1].y - comp_points[p2].y);

	    item.translate (delta, comp_points);
	  }
	}