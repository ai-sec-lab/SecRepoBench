// 0 when compressed

  if (compr_len > 0)
    error = decompress_r2007 ((BITCODE_RC *)file_header, sizeof (r2007_file_header),
                              &pedata[32], MIN (compr_len, pedatalength - 32));
  else
    memcpy (file_header, &pedata[32], sizeof (r2007_file_header));

  // check validity, for debugging only
  if (!error)
    {

#define VALID_SIZE(var)                                                       \
  if (var < 0 || (unsigned)var > dat->size)                                   \
    {                                                                         \
      errcount++;                                                             \
      error |= DWG_ERR_VALUEOUTOFBOUNDS;                                      \
      LOG_ERROR ("%s Invalid %s %ld > MAX_SIZE", __FUNCTION__, #var,          \
                 (long)var)                                                   \
      var = 0;                                                                \
    }
#define VALID_COUNT(var)                                                      \
  if (var < 0 || (unsigned)var > dat->size)                                   \
    {                                                                         \
      errcount++;                                                             \
      error |= DWG_ERR_VALUEOUTOFBOUNDS;                                      \
      LOG_ERROR ("%s Invalid %s %ld > MAX_COUNT", __FUNCTION__, #var,         \
                 (long)var)                                                   \
      var = 0;                                                                \
    }

      VALID_SIZE (file_header->header_size);
      VALID_SIZE (file_header->file_size);
      VALID_SIZE (file_header->pages_map_offset);
      VALID_SIZE (file_header->header2_offset);
      VALID_SIZE (file_header->pages_map_offset);
      VALID_SIZE (file_header->pages_map_size_comp);
      VALID_SIZE (file_header->pages_map_size_uncomp);
      VALID_COUNT (file_header->pages_maxid);
      VALID_COUNT (file_header->pages_amount);
      VALID_COUNT (file_header->num_sections);
    }

  free (pedata);
  return error;