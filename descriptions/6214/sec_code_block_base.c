if (length < 6)
    return(MagickFalse);
  exif=GetStringInfoDatum(profile);
  while (length != 0)
  {
    if (ReadPropertyByte(&exif,&length) != 0x45)
      continue;
    if (ReadPropertyByte(&exif,&length) != 0x78)
      continue;
    if (ReadPropertyByte(&exif,&length) != 0x69)
      continue;
    if (ReadPropertyByte(&exif,&length) != 0x66)
      continue;
    if (ReadPropertyByte(&exif,&length) != 0x00)
      continue;
    if (ReadPropertyByte(&exif,&length) != 0x00)
      continue;
    break;
  }
  if (length < 16)
    return(MagickFalse);
  id=(ssize_t) ReadPropertySignedShort(LSBEndian,exif);
  endian=LSBEndian;
  if (id == 0x4949)
    endian=LSBEndian;
  else
    if (id == 0x4D4D)
      endian=MSBEndian;
    else
      return(MagickFalse);
  if (ReadPropertyUnsignedShort(endian,exif+2) != 0x002a)
    return(MagickFalse);
  /*
    This the offset to the first IFD.
  */
  offset=(ssize_t) ReadPropertySignedLong(endian,exif+4);
  if ((offset < 0) || (size_t) offset >= length)
    return(MagickFalse);
  /*
    Set the pointer to the first IFD and follow it were it leads.
  */
  status=MagickFalse;
  directory=exif+offset;
  level=0;
  entry=0;
  tag_offset=0;
  exif_resources=NewSplayTree((int (*)(const void *,const void *)) NULL,
    (void *(*)(void *)) NULL,(void *(*)(void *)) NULL);
  do
  {
    /*
      If there is anything on the stack then pop it off.
    */
    if (level > 0)
      {
        level--;
        directory=directory_stack[level].directory;
        entry=directory_stack[level].entry;
        tag_offset=directory_stack[level].offset;
      }
    if ((directory < exif) || (directory > (exif+length-2)))
      break;
    /*
      Determine how many entries there are in the current IFD.
    */
    number_entries=(size_t) ReadPropertyUnsignedShort(endian,directory);
    for ( ; entry < number_entries; entry++)
    {
      register unsigned char
        *p,
        *q;

      size_t
        format;

      ssize_t
        number_bytes,
        components;

      q=(unsigned char *) (directory+(12*entry)+2);
      if (q > (exif+length-12))
        break;  /* corrupt EXIF */
      if (GetValueFromSplayTree(exif_resources,q) == q)
        break;
      (void) AddValueToSplayTree(exif_resources,q,q);
      tag_value=(size_t) ReadPropertyUnsignedShort(endian,q)+tag_offset;
      format=(size_t) ReadPropertyUnsignedShort(endian,q+2);
      if (format >= (sizeof(tag_bytes)/sizeof(*tag_bytes)))
        break;
      components=(ssize_t) ReadPropertySignedLong(endian,q+4);
      if (components < 0)
        break;  /* corrupt EXIF */
      number_bytes=(size_t) components*tag_bytes[format];
      if (number_bytes < components)
        break;  /* prevent overflow */
      if (number_bytes <= 4)
        p=q+8;
      else
        {
          ssize_t
            offset;

          /*
            The directory entry contains an offset.
          */
          offset=(ssize_t) ReadPropertySignedLong(endian,q+8);
          if ((offset < 0) || (size_t) offset >= length)
            continue;
          if ((ssize_t) (offset+number_bytes) < offset)
            continue;  /* prevent overflow */
          if ((size_t) (offset+number_bytes) > length)
            continue;
          p=(unsigned char *) (exif+offset);
        }
      if ((all != 0) || (tag == (size_t) tag_value))
        {
          char
            buffer[MagickPathExtent],
            *value;

          value=(char *) NULL;
          *buffer='\0';
          switch (format)
          {
            case EXIF_FMT_BYTE:
            case EXIF_FMT_UNDEFINED:
            {
              EXIFMultipleValues(1,"%.20g",(double) (*(unsigned char *) p1));
              break;
            }
            case EXIF_FMT_SBYTE:
            {
              EXIFMultipleValues(1,"%.20g",(double) (*(signed char *) p1));
              break;
            }
            case EXIF_FMT_SSHORT:
            {
              EXIFMultipleValues(2,"%hd",ReadPropertySignedShort(endian,p1));
              break;
            }
            case EXIF_FMT_USHORT:
            {
              EXIFMultipleValues(2,"%hu",ReadPropertyUnsignedShort(endian,p1));
              break;
            }
            case EXIF_FMT_ULONG:
            {
              EXIFMultipleValues(4,"%.20g",(double)
                ReadPropertyUnsignedLong(endian,p1));
              break;
            }
            case EXIF_FMT_SLONG:
            {
              EXIFMultipleValues(4,"%.20g",(double)
                ReadPropertySignedLong(endian,p1));
              break;
            }
            case EXIF_FMT_URATIONAL:
            {
              EXIFMultipleFractions(8,"%.20g/%.20g",(double)
                ReadPropertyUnsignedLong(endian,p1),(double)
                ReadPropertyUnsignedLong(endian,p1+4));
              break;
            }
            case EXIF_FMT_SRATIONAL:
            {
              EXIFMultipleFractions(8,"%.20g/%.20g",(double)
                ReadPropertySignedLong(endian,p1),(double)
                ReadPropertySignedLong(endian,p1+4));
              break;
            }
            case EXIF_FMT_SINGLE:
            {
              EXIFMultipleValues(4,"%f",(double) *(float *) p1);
              break;
            }
            case EXIF_FMT_DOUBLE:
            {
              EXIFMultipleValues(8,"%f",*(double *) p1);
              break;
            }
            default:
            case EXIF_FMT_STRING:
            {
              value=(char *) NULL;
              if (~((size_t) number_bytes) >= 1)
                value=(char *) AcquireQuantumMemory((size_t) number_bytes+1UL,
                  sizeof(*value));
              if (value != (char *) NULL)
                {
                  register ssize_t
                    i;

                  for (i=0; i < (ssize_t) number_bytes; i++)
                  {
                    value[i]='.';
                    if ((isprint((int) p[i]) != 0) || (p[i] == '\0'))
                      value[i]=(char) p[i];
                  }
                  value[i]='\0';
                }
              break;
            }
          }
          if (value != (char *) NULL)
            {
              char
                *key;

              register const char
                *p;

              key=AcquireString(property);
              switch (all)
              {
                case 1:
                {
                  const char
                    *description;

                  register ssize_t
                    i;

                  description="unknown";
                  for (i=0; ; i++)
                  {
                    if (EXIFTag[i].tag == 0)
                      break;
                    if (EXIFTag[i].tag == tag_value)
                      {
                        description=EXIFTag[i].description;
                        break;
                      }
                  }
                  (void) FormatLocaleString(key,MagickPathExtent,"%s",
                    description);
                  if (level == 2)
                    (void) SubstituteString(&key,"exif:","exif:thumbnail:");
                  break;
                }
                case 2:
                {
                  if (tag_value < 0x10000)
                    (void) FormatLocaleString(key,MagickPathExtent,"#%04lx",
                      (unsigned long) tag_value);
                  else
                    if (tag_value < 0x20000)
                      (void) FormatLocaleString(key,MagickPathExtent,"@%04lx",
                        (unsigned long) (tag_value & 0xffff));
                    else
                      (void) FormatLocaleString(key,MagickPathExtent,"unknown");
                  break;
                }
                default:
                {
                  if (level == 2)
                    (void) SubstituteString(&key,"exif:","exif:thumbnail:");
                }
              }
              p=(const char *) NULL;
              if (image->properties != (void *) NULL)
                p=(const char *) GetValueFromSplayTree((SplayTreeInfo *)
                  image->properties,key);
              if (p == (const char *) NULL)
                (void) SetImageProperty((Image *) image,key,value,exception);
              value=DestroyString(value);
              key=DestroyString(key);
              status=MagickTrue;
            }
        }
        if ((tag_value == TAG_EXIF_OFFSET) ||
            (tag_value == TAG_INTEROP_OFFSET) || (tag_value == TAG_GPS_OFFSET))
          {
            ssize_t
              offset;

            offset=(ssize_t) ReadPropertySignedLong(endian,p);
            if (((size_t) offset < length) && (level < (MaxDirectoryStack-2)))
              {
                ssize_t
                  tag_offset1;

                tag_offset1=(ssize_t) ((tag_value == TAG_GPS_OFFSET) ? 0x10000 :
                  0);
                directory_stack[level].directory=directory;
                entry++;
                directory_stack[level].entry=entry;
                directory_stack[level].offset=tag_offset;
                level++;
                directory_stack[level].directory=exif+offset;
                directory_stack[level].offset=tag_offset1;
                directory_stack[level].entry=0;
                level++;
                if ((directory+2+(12*number_entries)) > (exif+length))
                  break;
                offset=(ssize_t) ReadPropertySignedLong(endian,directory+2+(12*
                  number_entries));
                if ((offset != 0) && ((size_t) offset < length) &&
                    (level < (MaxDirectoryStack-2)))
                  {
                    directory_stack[level].directory=exif+offset;
                    directory_stack[level].entry=0;
                    directory_stack[level].offset=tag_offset1;
                    level++;
                  }
              }
            break;
          }
    }
  } while (level > 0);
  exif_resources=DestroySplayTree(exif_resources);
  return(status);