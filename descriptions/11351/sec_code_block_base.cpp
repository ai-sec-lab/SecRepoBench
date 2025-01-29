while (initial_gids_to_retain->next (&gid))
  {
    _add_gid_and_children (glyf, gid, all_gids_to_retain);
  }
  hb_set_destroy (initial_gids_to_retain);

  _remove_invalid_gids (all_gids_to_retain, face->get_num_glyphs ());

  glyphs->alloc (all_gids_to_retain->get_population ());
  gid = HB_SET_VALUE_INVALID;