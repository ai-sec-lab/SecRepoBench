  MagickOffsetType
    offset;

  assert(exception != (ExceptionInfo *) NULL);
  if (IsMagicListInstantiated(exception) == MagickFalse)
    return((const MagicInfo *) NULL);
  if (IsMagicCacheInstantiated() == MagickFalse)
    return((const MagicInfo *) NULL);
  /*
    Search for cached entries.
  */
  if (magic != (const unsigned char *) NULL)
    {
      LockSemaphoreInfo(magic_cache_semaphore);
      ResetLinkedListIterator(magic_cache);
      p=(const MagicInfo *) GetNextValueInLinkedList(magic_cache);
      while (p != (const MagicInfo *) NULL)
      {
        const unsigned char
          *q;

        q=magic;
        if (p->skip_spaces != MagickFalse)
          while (isspace(*q) != 0) q++;
        assert(p->offset >= 0);
        offset=p->offset+(MagickOffsetType) p->length;
        if ((offset <= (MagickOffsetType) size) &&
            (memcmp(q+p->offset,p->magic,p->length) == 0))
          break;
        p=(const MagicInfo *) GetNextValueInLinkedList(magic_cache);
      }
      UnlockSemaphoreInfo(magic_cache_semaphore);
      if (p != (const MagicInfo *) NULL)
        return(p);
    }
  /*
    Search for magic tag.
  */
  LockSemaphoreInfo(magic_list_semaphore);
  ResetLinkedListIterator(magic_list);
  p=(const MagicInfo *) GetNextValueInLinkedList(magic_list);
  if (magic == (const unsigned char *) NULL)
    {
      UnlockSemaphoreInfo(magic_list_semaphore);
      return(p);
    }
  while (p != (const MagicInfo *) NULL)
  {
    const unsigned char
      *q;

    q=magic;
    if (p->skip_spaces != MagickFalse)
      while (isspace(*q) != 0) q++;
    assert(p->offset >= 0);
    offset=p->offset+(MagickOffsetType) p->length;
    if ((offset <= (MagickOffsetType) size) &&
        (memcmp(q+p->offset,p->magic,p->length) == 0))
      break;
    p=(const MagicInfo *) GetNextValueInLinkedList(magic_list);
  }