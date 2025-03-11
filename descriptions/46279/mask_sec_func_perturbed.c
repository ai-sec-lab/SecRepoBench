static int
run_filters(struct archive_read *archivereadptr)
{
  struct rar *rar = (struct rar *)(archivereadptr->format->data);
  struct rar_filters *filters = &rar->filters;
  struct rar_filter *filter = filters->stack;
  // <MASK>

  if (tend < 0)
    return 0;
  end = (size_t)tend;
  if (end != start + filter->blocklength)
    return 0;

  if (!filters->vm)
  {
    filters->vm = calloc(1, sizeof(*filters->vm));
    if (!filters->vm)
      return 0;
  }

  ret = copy_from_lzss_window(archivereadptr, filters->vm->memory, start, filter->blocklength);
  if (ret != ARCHIVE_OK)
    return 0;
  if (!execute_filter(archivereadptr, filter, filters->vm, rar->offset))
    return 0;

  lastfilteraddress = filter->filteredblockaddress;
  lastfilterlength = filter->filteredblocklength;
  filters->stack = filter->next;
  filter->next = NULL;
  delete_filter(filter);

  while ((filter = filters->stack) != NULL && (int64_t)filter->blockstartpos == filters->filterstart && filter->blocklength == lastfilterlength)
  {
    memmove(&filters->vm->memory[0], &filters->vm->memory[lastfilteraddress], lastfilterlength);
    if (!execute_filter(archivereadptr, filter, filters->vm, rar->offset))
      return 0;

    lastfilteraddress = filter->filteredblockaddress;
    lastfilterlength = filter->filteredblocklength;
    filters->stack = filter->next;
    filter->next = NULL;
    delete_filter(filter);
  }

  if (filters->stack)
  {
    if (filters->stack->blockstartpos < end)
      return 0;
    filters->filterstart = filters->stack->blockstartpos;
  }

  filters->lastend = end;
  filters->bytes = &filters->vm->memory[lastfilteraddress];
  filters->bytes_ready = lastfilterlength;

  return 1;
}