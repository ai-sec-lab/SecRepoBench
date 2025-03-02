TRACE_SERIALIZE (this);
    VarRegionList *out = c->allocate_min<VarRegionList> ();
    if (unlikely (!out)) return_trace (false);
    axisCount = src->axisCount;
    regionCount = region_map.get_population ();
    if (unlikely (!c->allocate_size<VarRegionList> (get_size () - min_size))) return_trace (false);
    for (unsigned int r = 0; r < regionCount; r++)
      memcpy (&axesZ[axisCount * r], &src->axesZ[axisCount * region_map.backward (r)], VarRegionAxis::static_size * axisCount);

    return_trace (true);