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
        // Allocate memory and copy the string from 'value' to a new memory block,
        // accounting for both fixed-length and dynamically allocated strings.
        // If the string is fixed-length (TFv), allocate memory according to the field's size
        // and copy the string with consideration for null-termination.
        // For ASCII strings with specific DWG versions, allocate memory based on
        // the length of the input string and copy it accordingly.
        // For wide character strings, convert UTF-8 encoded strings to a wide character format
        // if necessary, and allocate memory for the wide string before copying it.
        // If the field type is not TF and DWG version conditions are met, perform these conversions.
        // In all cases, update the pointer in 'old' to the newly allocated string memory,
        // ensuring the previous memory is freed to avoid memory leaks.
        // <MASK>
    }
  else
    memcpy (old, value, f->size);
}