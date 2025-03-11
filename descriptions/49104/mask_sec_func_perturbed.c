MagickExport const MagicInfo *GetMagicInfo(const unsigned char *magic,
  const size_t size,ExceptionInfo *exception)
{
  const MagicInfo
    *p;

  // <MASK>
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