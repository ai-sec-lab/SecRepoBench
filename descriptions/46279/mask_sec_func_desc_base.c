static int
run_filters(struct archive_read *a)
{
  struct rar *rar = (struct rar *)(a->format->data);
  struct rar_filters *filters = &rar->filters;
  struct rar_filter *filter = filters->stack;
  // Check if filters or the current filter is NULL, returning 0 if so, indicating failure.
  // Set the starting position for the filter and calculate the end position.
  // Update the filter's starting position to a maximum value, suggesting a reset or
  // invalidation of the current filter start.
  // Attempt to expand the archive read buffer up to the calculated end position.
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

  ret = copy_from_lzss_window(a, filters->vm->memory, start, filter->blocklength);
  if (ret != ARCHIVE_OK)
    return 0;
  if (!execute_filter(a, filter, filters->vm, rar->offset))
    return 0;

  lastfilteraddress = filter->filteredblockaddress;
  lastfilterlength = filter->filteredblocklength;
  filters->stack = filter->next;
  filter->next = NULL;
  delete_filter(filter);

  while ((filter = filters->stack) != NULL && (int64_t)filter->blockstartpos == filters->filterstart && filter->blocklength == lastfilterlength)
  {
    memmove(&filters->vm->memory[0], &filters->vm->memory[lastfilteraddress], lastfilterlength);
    if (!execute_filter(a, filter, filters->vm, rar->offset))
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