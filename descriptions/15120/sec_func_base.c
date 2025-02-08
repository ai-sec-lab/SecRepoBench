static int
archive_read_format_rar_read_data(struct archive_read *a, const void **buff,
                                  size_t *size, int64_t *offset)
{
  struct rar *rar = (struct rar *)(a->format->data);
  int ret;

  if (rar->has_encrypted_entries == ARCHIVE_READ_FORMAT_ENCRYPTION_DONT_KNOW) {
	  rar->has_encrypted_entries = 0;
  }

  if (rar->bytes_unconsumed > 0) {
      /* Consume as much as the decompressor actually used. */
      __archive_read_consume(a, rar->bytes_unconsumed);
      rar->bytes_unconsumed = 0;
  }

  *buff = NULL;
  if (rar->entry_eof || rar->offset_seek >= rar->unp_size) {
    *size = 0;
    *offset = rar->offset;
    if (*offset < rar->unp_size)
      *offset = rar->unp_size;
    return (ARCHIVE_EOF);
  }

  switch (rar->compression_method)
  {
  case COMPRESS_METHOD_STORE:
    ret = read_data_stored(a, buff, size, offset);
    break;

  case COMPRESS_METHOD_FASTEST:
  case COMPRESS_METHOD_FAST:
  case COMPRESS_METHOD_NORMAL:
  case COMPRESS_METHOD_GOOD:
  case COMPRESS_METHOD_BEST:
    ret = read_data_compressed(a, buff, size, offset);
    if (ret != ARCHIVE_OK && ret != ARCHIVE_WARN) {
      __archive_ppmd7_functions.Ppmd7_Free(&rar->ppmd7_context);
      rar->start_new_table = 1;
      rar->ppmd_valid = 0;
    }
    break;

  default:
    archive_set_error(&a->archive, ARCHIVE_ERRNO_FILE_FORMAT,
                      "Unsupported compression method for RAR file.");
    ret = ARCHIVE_FATAL;
    break;
  }
  return (ret);
}