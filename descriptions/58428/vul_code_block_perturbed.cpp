const HBUINT16 *endPtsOfContours = &StructAfter<HBUINT16> (header);
    int num_contours = header.numberOfContours;
    assert (num_contours > 0);
    /* One extra item at the end, for the instruction-count below. */
    if (unlikely (!bytes.check_range (&endPtsOfContours[num_contours]))) return false;
    unsigned int num_points = endPtsOfContours[num_contours - 1] + 1;

    contour_points.alloc (num_points + 4, true); // Allocate for phantom points, to avoid a possible copy
    if (!contour_points.resize (num_points, false)) return false;
    if (phantom_only) return true;

    for (unsigned i = 0; i < contour_points.length; i++)
      contour_points.arrayZ[i].is_end_point = false;
    for (int i = 0; i < num_contours; i++)
      contour_points[endPtsOfContours[i]].is_end_point = true;

    /* Skip instructions */