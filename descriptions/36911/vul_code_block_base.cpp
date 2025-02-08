sorted_entries.qsort (compare_entries);
  bool ret = f->serialize_single (&c, sfnt_tag, + sorted_entries.iter());

  c.end_serialize ();

  if (unlikely (!ret))
  {
    hb_free (buf);
    return nullptr;
  }

  return hb_blob_create (buf, face_length, HB_MEMORY_MODE_WRITABLE, buf, hb_free);