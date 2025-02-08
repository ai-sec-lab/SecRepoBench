static hb_blob_t *
_hb_face_builder_data_reference_blob (hb_face_builder_data_t *data)
{

  unsigned int table_count = data->tables.get_population ();
  unsigned int face_length = table_count * 16 + 12;

  for (hb_blob_t* b : data->tables.values())
    face_length += hb_ceil_to_4 (hb_blob_get_length (b));

  char *buf = (char *) hb_malloc (face_length);
  if (unlikely (!buf))
    return nullptr;

  hb_serialize_context_t c (buf, face_length);
  c.propagate_error (data->tables);
  OT::OpenTypeFontFile *f = c.start_serialize<OT::OpenTypeFontFile> ();

  bool is_cff = (data->tables.has (HB_TAG ('C','F','F',' '))
                 || data->tables.has (HB_TAG ('C','F','F','2')));
  hb_tag_t fonttag = is_cff ? OT::OpenTypeFontFile::CFFTag : OT::OpenTypeFontFile::TrueTypeTag;

  // Sort the tags so that produced face is deterministic.
  hb_vector_t<hb_pair_t <hb_tag_t, hb_blob_t*>> sorted_entries;
  data->tables.iter () | hb_sink (sorted_entries);
  if (unlikely (sorted_entries.in_error ()))
  {
    hb_free (buf);
    return nullptr;
  }

  sorted_entries.qsort (compare_entries);
  bool ret = f->serialize_single (&c, fonttag, + sorted_entries.iter());

  c.end_serialize ();

  if (unlikely (!ret))
  {
    hb_free (buf);
    return nullptr;
  }

  return hb_blob_create (buf, face_length, HB_MEMORY_MODE_WRITABLE, buf, hb_free);
}