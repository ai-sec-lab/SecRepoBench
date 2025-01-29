if (!points_.resize (num_points)) return false;
    if (phantom_only) return true;

    for (int i = 0; i < num_contours; i++)
      points_[endPtsOfContours[i]].is_end_point = true;