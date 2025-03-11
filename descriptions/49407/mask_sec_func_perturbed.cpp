void map_coords (int *coords, unsigned int coords_length) const
  {
    unsigned int effectiveaxiscount = hb_min (coords_length, axisCount);

    const SegmentMaps *map = &firstAxisSegmentMaps;
    // <MASK>
  }