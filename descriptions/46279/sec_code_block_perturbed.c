struct rar_filter *f;
  size_t start, end;
  int64_t tend;
  uint32_t lastfilteraddress;
  uint32_t lastfilterlength;
  int ret;

  if (filters == NULL || filter == NULL)
    return (0);

  start = filters->filterstart;
  end = start + filter->blocklength;

  filters->filterstart = INT64_MAX;
  tend = (int64_t)end;
  ret = expand(archivereadptr, &tend);
  if (ret != ARCHIVE_OK)
    return 0;

  /* Check if filter stack was modified in expand() */
  ret = ARCHIVE_FATAL;
  f = filters->stack;
  while (f)
  {
    if (f == filter)
    {
      ret = ARCHIVE_OK;
      break;
    }
    f = f->next;
  }
  if (ret != ARCHIVE_OK)
    return 0;