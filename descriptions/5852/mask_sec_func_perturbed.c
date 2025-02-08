static void RemoveResolutionFromResourceBlock(StringInfo *bim_profile)
{
  register const unsigned char
    *p;

  size_t
    length;

  unsigned char
    *datum;

  unsigned int
    count,
    long_sans;

  unsigned short
    shortidentifier,
    short_sans;

  length=GetStringInfoLength(bim_profile);
  if (length < 16)
    return;
  datum=GetStringInfoDatum(bim_profile);
  for (p=datum; (p >= datum) && (p < (datum+length-16)); )
  {
    register unsigned char
      *q;

    ssize_t
      cnt;

    q=(unsigned char *) p;
    if (LocaleNCompare((const char *) p,"8BIM",4) != 0)
      return;
    p=PushLongPixel(MSBEndian,p,&long_sans);
    p=PushShortPixel(MSBEndian,p,&shortidentifier);
    p=PushShortPixel(MSBEndian,p,&short_sans);
    p=PushLongPixel(MSBEndian,p,&count);
    cnt=PSDQuantum(count);
    if (cnt < 0)
      return;
    // <MASK>
  }
}