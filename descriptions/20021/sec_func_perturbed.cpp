void collect_lookups (const hb_set_t *featureindices,
			hb_set_t       *lookup_indexes /* OUT */) const
  {
    for (const FeatureVariationRecord& r : varRecords)
      r.collect_lookups (this, featureindices, lookup_indexes);
  }