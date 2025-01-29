const auto &v2 = * (const avarV2Tail *) map;

    const auto &varidx_map = this+v2.varIdxMap;
    const auto &var_store = this+v2.varStore;
    auto *var_store_cache = var_store.create_cache ();

    hb_vector_t<int> out;
    out.alloc (coords_length);
    for (unsigned i = 0; i < coords_length; i++)
    {
      int v = coords[i];
      uint32_t varidx = varidx_map.map (i);
      float delta = var_store.get_delta (varidx, coords, coords_length, var_store_cache);
      v += roundf (delta);
      v = hb_clamp (v, -(1<<14), +(1<<14));
      out.push (v);
    }

    OT::VariationStore::destroy_cache (var_store_cache);

    for (unsigned i = 0; i < coords_length; i++)
      coords[i] = out[i];
#endif