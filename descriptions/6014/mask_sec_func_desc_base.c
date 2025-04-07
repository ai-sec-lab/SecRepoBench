static char *ParseEntities(char *xml,char **entities,int state)
{
  char
    *entity;

  int
    byte,
    c;

  register char
    *p,
    *q;

  register ssize_t
    i;

  size_t
    extent,
    length;

  ssize_t
    offset;

  /*
    Normalize line endings.
  */
  p=xml;
  q=xml;
  for ( ; *xml != '\0'; xml++)
    while (*xml == '\r')
    {
      *(xml++)='\n';
      if (*xml == '\n')
        (void) CopyMagickMemory(xml,xml+1,strlen(xml));
    }
  for (xml=p; ; )
  {
    while ((*xml != '\0') && (*xml != '&') && ((*xml != '%') ||
           (state != '%')) && (isspace((int) ((unsigned char) *xml) == 0)))
      xml++;
    if (*xml == '\0')
      break;
    /*
      States include:
        '&' for general entity decoding
        '%' for parameter entity decoding
        'c' for CDATA sections
        ' ' for attributes normalization
        '*' for non-CDATA attributes normalization
    */
    if ((state != 'c') && (strncmp(xml,"&#",2) == 0))
      {
        /*
          Character reference.
        */
        if (xml[2] != 'x')
          c=strtol(xml+2,&entity,10);  /* base 10 */
        else
          c=strtol(xml+3,&entity,16);  /* base 16 */
        if ((c == 0) || (*entity != ';'))
          {
            /*
              Not a character reference.
            */
            xml++;
            continue;
          }
        if (c < 0x80)
          *(xml++)=c;
        else
          {
            /*
              Multi-byte UTF-8 sequence.
            */
            byte=c;
            for (i=0; byte != 0; byte/=2)
              i++;
            i=(i-2)/5;
            *xml=(char) ((0xFF << (7-i)) | (c >> (6*i)));
            xml++;
            while (i != 0)
            {
              i--;
              *xml=(char) (0x80 | ((c >> (6*i)) & 0x3F));
              xml++;
            }
          }
        (void) CopyMagickMemory(xml,strchr(xml,';')+1,strlen(strchr(xml,';')));
      }
    else
      if (((*xml == '&') && ((state == '&') || (state == ' ') ||
          (state == '*'))) || ((state == '%') && (*xml == '%')))
        {
          /*
            Find entity in the list.
          */
          i=0;
          while ((entities[i] != (char *) NULL) &&
                 (strncmp(xml+1,entities[i],strlen(entities[i])) != 0))
            i+=2;
          if (entities[i++] == (char *) NULL)
            xml++;
          else
            if (entities[i] != (char *) NULL)
              {
                /*
                  Found a match.
                */
                length=strlen(entities[i]);
                entity=strchr(xml,';');
                if ((entity != (char *) NULL) &&
                    ((length-1L) >= (size_t) (entity-xml)))
                  {
                    offset=(ssize_t) (xml-p);
                    extent=(size_t) (offset+length+strlen(entity));
                    if (p != q)
                      p=(char *) ResizeQuantumMemory(p,extent,sizeof(*p));
                    else
                      {
                        char
                          *extent_xml;

                        extent_xml=(char *) AcquireQuantumMemory(extent,
                          sizeof(*extent_xml));
                        if (extent_xml != (char *) NULL)
                          {
                            ResetMagickMemory(extent_xml,0,extent*
                              sizeof(*extent_xml));
                            (void) CopyMagickString(extent_xml,p,extent*
                              sizeof(*extent_xml));
                          }
                        p=extent_xml;
                      }
                    if (p == (char *) NULL)
                      ThrowFatalException(ResourceLimitFatalError,
                        "MemoryAllocationFailed");
                    xml=p+offset;
                    entity=strchr(xml,';');
                  }
                if (entity != (char *) NULL)
                  (void) CopyMagickMemory(xml+length,entity+1,strlen(entity));
                (void) strncpy(xml,entities[i],length);
              }
        }
      else
        if (((state == ' ') || (state == '*')) &&
            (isspace((int) ((unsigned char) *xml) != 0)))
          *(xml++)=' ';
        else
          xml++;
  }
  if (state == '*')
    {
      /*
        Normalize spaces for non-CDATA attributes.
      */
      // Iterate over the normalized string to remove leading spaces from each word.
      // Use a character pointer to traverse through the string.
      // Use a loop to skip over and remove consecutive spaces.
      // Compact the string by shifting non-space characters to the left.
      // Continue this until the end of the string is reached.
      // If the last character is a space, terminate the string before the space.
      // <MASK>
    }
  return(p == q ? ConstantString(p) : p);
}