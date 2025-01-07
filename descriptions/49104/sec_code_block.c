
  const unsigned char
    *q;

  MagickOffsetType
    remaining;

  assert(magic_info->offset >= 0);
  q=magic+magic_info->offset;
  remaining=(MagickOffsetType) length-magic_info->offset;
  if (magic_info->skip_spaces != MagickFalse)
    {
      while ((remaining > 0) && (isspace(*q) != 0))
        {
          q++;
          remaining--;
        }
    }
  if ((remaining >= magic_info->length) &&
      (memcmp(q,magic_info->magic,magic_info->length) == 0))
    return(MagickTrue);
  return(MagickFalse);
