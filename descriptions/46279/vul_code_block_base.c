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
  ret = expand(a, &tend);
  if (ret != ARCHIVE_OK)
    return 0;