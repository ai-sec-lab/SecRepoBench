MagickExport const MagicInfo *GetMagicInfo(const unsigned char *magic,
  const size_t size,ExceptionInfo *exception)
{
  const MagicInfo
    *p;

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
        if (MatchesMagic(magic,size,p) != MagickFalse)
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
    if (MatchesMagic(magic,size,p) != MagickFalse)
      break;
    p=(const MagicInfo *) GetNextValueInLinkedList(magic_list);
  }
  UnlockSemaphoreInfo(magic_list_semaphore);
  if (p != (const MagicInfo *) NULL)
    {
      LockSemaphoreInfo(magic_cache_semaphore);
      (void) InsertValueInSortedLinkedList(magic_cache,CompareMagickInfoExtent,
        NULL,p);
      UnlockSemaphoreInfo(magic_cache_semaphore);
    }
  return(p);
}