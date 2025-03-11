end:
  if (fp != NULL) {
    fclose(fp);
  }
  if (rc < 0) {
    if (*needs_free) {
      free(*chunk);
      *chunk = NULL;
    }
    return rc;
  }

  return (int)lazy_chunk_bytes;