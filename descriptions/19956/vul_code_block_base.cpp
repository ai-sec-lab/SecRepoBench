for (unsigned int r = 0; r < regionCount; r++)
      memcpy (&axesZ[axisCount * r], &src->axesZ[axisCount * region_map.backward (r)], VarRegionAxis::static_size * axisCount);

    return_trace (true);