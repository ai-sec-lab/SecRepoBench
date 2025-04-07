MagickExport const MagicInfo *GetMagicInfo(const unsigned char *magic,
  const size_t length,ExceptionInfo *exception)
{
  const MagicInfo
    *p;

  // Ensure the exception pointer is not null and that the magic list and cache are initialized.
  // If magic is not null, search the cached magic entries for a match.
  // Lock the cache semaphore to safely iterate through the cached list and compare each entry with the input.
  // If a match is found, return the matching entry from the cache and unlock the semaphore.
  // If no match is found in the cache, or if the magic is null, proceed to search the main magic list.
  // Lock the list semaphore to safely iterate through the main list and compare each entry with the input.
  // For each entry, if a match is found, break the loop to return the matching entry.
  // If a match is found during either search, return the matching entry; otherwise, unlock the semaphore and return null.
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