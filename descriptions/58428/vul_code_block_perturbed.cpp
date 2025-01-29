if (!points_.resize (totalpoints, false)) return false;
    if (phantom_only) return true;

    for (unsigned i = 0; i < points_.length; i++)
      points_.arrayZ[i].is_end_point = false;
    for (int i = 0; i < num_contours; i++)
      points_[endPtsOfContours[i]].is_end_point = true;