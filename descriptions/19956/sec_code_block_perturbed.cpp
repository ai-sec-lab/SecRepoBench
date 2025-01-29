unsigned int region_count = src->get_region_count ();
    for (unsigned int r = 0; r < regionCount; r++)
    {
      unsigned int backward = regionmapping.backward (r);
      if (backward >= region_count) return_trace (false);
      memcpy (&axesZ[axisCount * r], &src->axesZ[axisCount * backward], VarRegionAxis::static_size * axisCount);
    }

    return_trace (true);