bool get_contour_points (contour_point_vector_t &points_ /* OUT */,
			   bool phantom_only = false) const
  {
    // <MASK>
    const HBUINT8 *p = &StructAtOffset<HBUINT8> (&endPtsOfContours[num_contours + 1],
						 endPtsOfContours[num_contours]);

    if (unlikely ((const char *) p < bytes.arrayZ)) return false; /* Unlikely overflow */
    const HBUINT8 *end = (const HBUINT8 *) (bytes.arrayZ + bytes.length);
    if (unlikely (p >= end)) return false;

    /* Read x & y coordinates */
    return read_flags (p, points_, end)
        && read_points (p, points_, end, &contour_point_t::x,
			FLAG_X_SHORT, FLAG_X_SAME)
	&& read_points (p, points_, end, &contour_point_t::y,
			FLAG_Y_SHORT, FLAG_Y_SAME);
  }