static void
dynapi_set_helper (void *restrict old, const Dwg_DYNAPI_field *restrict f,
                   const Dwg_Version_Type versiontype,
                   const void *restrict value, const bool is_utf8)
{
  // TODO: sanity checks. is_malloc (TF), copy zero's (TFv)
  // if text strcpy or wcscpy, or do utf8 conversion.
  //if ((char*)old && f->is_malloc)
  //  free (old);
  if (f->is_malloc)
    {
      // NULL ptr
      if (!*(char**)value)
        memcpy (old, value, sizeof (char*));
      // fixed length (but not yet TF)
      else if (strEQc (f->type, "TFv"))
        // <MASK>
    }
  else
    memcpy (old, value, f->size);
}