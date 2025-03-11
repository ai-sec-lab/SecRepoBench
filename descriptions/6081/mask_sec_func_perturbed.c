static MagickBooleanType ParseInternalDoctype(XMLTreeRoot *root,char *xml,
  size_t length,ExceptionInfo *exception)
{
  char
    *c,
    **entities,
    *n,
    **predefined_entitites,
    q,
    *t,
    *v;

  register ssize_t
    i;

  ssize_t
    index;

  n=(char *) NULL;
  predefined_entitites=(char **) AcquireMagickMemory(sizeof(sentinel));
  if (predefined_entitites == (char **) NULL)
    ThrowFatalException(ResourceLimitError,"MemoryAllocationFailed");
  (void) CopyMagickMemory(predefined_entitites,sentinel,sizeof(sentinel));
  for (xml[length]='\0'; xml != (char *) NULL; )
  {
    // <MASK>
    }
  predefined_entitites=(char **) RelinquishMagickMemory(predefined_entitites);
  return(MagickTrue);
}