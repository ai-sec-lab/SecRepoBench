void collect_lookups (const hb_set_t *feature_indexes,
			hb_set_t       *lookup_indexes /* OUT */) const
  {
    for (const FeatureVariationRecord& r : varRecords)
      r.collect_lookups (this, feature_indexes, lookup_indexes);
  }