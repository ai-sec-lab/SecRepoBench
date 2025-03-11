while (initial_gids_to_retain->next (&gid))
  {
    _add_gid_and_children (glyf, gid, full_glyphs_to_retain);
  }
  hb_set_destroy (initial_gids_to_retain);

  glyphs->alloc (full_glyphs_to_retain->get_population ());
  gid = HB_SET_VALUE_INVALID;
  while (full_glyphs_to_retain->next (&gid))
    glyphs->push (gid);

  glyf.fini ();
  cmap.fini ();

  return full_glyphs_to_retain;