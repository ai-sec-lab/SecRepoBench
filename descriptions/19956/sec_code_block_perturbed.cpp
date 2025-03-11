TRACE_SERIALIZE (this);
    VarRegionList *out = c->allocate_min<VarRegionList> ();
    if (unlikely (!out)) return_trace (false);
    axisCount = src->axisCount;
    regionCount = regionmapping.get_population ();
    if (unlikely (!c->allocate_size<VarRegionList> (get_size () - min_size))) return_trace (false);
    unsigned int region_count = src->get_region_count ();
    for (unsigned int r = 0; r < regionCount; r++)
    {
      unsigned int backward = regionmapping.backward (r);
      if (backward >= region_count) return_trace (false);
      memcpy (&axesZ[axisCount * r], &src->axesZ[axisCount * backward], VarRegionAxis::static_size * axisCount);
    }

    return_trace (true);