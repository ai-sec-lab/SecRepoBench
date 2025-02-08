void dotnet_parse_us(
    PE* pe,
    int64_t metadata_root,
    PSTREAM_HEADER us_header)
{
  BLOB_PARSE_RESULT blob_result;
  int i = 0;

  const uint8_t* offset = pe->data + metadata_root + us_header->Offset;
  const uint8_t* end_of_header = offset + us_header->Size;

  // Make sure the header size is larger than 0 and its end is not past the
  // end of PE.
  if (us_header->Size == 0 || !fits_in_pe(pe, offset, us_header->Size))
    return;

  // The first entry MUST be single NULL byte.
  if (*offset != 0x00)
    return;

  offset++;

  while (offset < end_of_header)
  {
    blob_result = dotnet_parse_blob_entry(pe, offset);

    if (blob_result.size == 0 || !fits_in_pe(pe, offset, blob_result.length))
    {
      set_integer(i, pe->object, "number_of_user_strings");
      return;
    }

    offset += blob_result.size;
    // Avoid empty strings, which usually happen as padding at the end of the
    // stream.

    if (blob_result.length > 0)
    {
      set_sized_string(
         (char*) offset,
         blob_result.length,
         pe->object,
         "user_strings[%i]",
         i);

      offset += blob_result.length;
      i++;
    }
  }

  set_integer(i, pe->object, "number_of_user_strings");
}