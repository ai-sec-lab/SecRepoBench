((all != 0) || (tag == (size_t) tag_value))
        {
          char
            buffer[6*sizeof(double)+MagickPathExtent],
            *value;

          if ((p < exif) || (p > (exif+length-tag_bytes[format])))
            break;
          value=(char *) NULL;
          *buffer='\0';
          switch (format)
          {
            case EXIF_FMT_BYTE:
            {
              value=(char *) NULL;
              if (~((size_t) number_bytes) >= 1)
                value=(char *) AcquireQuantumMemory((size_t) number_bytes+1UL,
                  sizeof(*value));
              if (value != (char *) NULL)
                {
                  for (i=0; i < (ssize_t) number_bytes; i++)
                  {
                    value[i]='.';
                    if (isprint((int) p[i]) != 0)
                      value[i]=(char) p[i];
                  }
                  value[i]='\0';
                }
              break;
            }
            case EXIF_FMT_SBYTE:
            {
              EXIFMultipleValues("%.20g",(double) (*(signed char *) p));
              break;
            }
            case EXIF_FMT_SSHORT:
            {
              EXIFMultipleValues("%hd",ReadPropertySignedShort(endian,p));
              break;
            }
            case EXIF_FMT_USHORT:
            {
              EXIFMultipleValues("%hu",ReadPropertyUnsignedShort(endian,p));
              break;
            }
            case EXIF_FMT_ULONG:
            {
              EXIFMultipleValues("%.20g",(double)
                ReadPropertyUnsignedLong(endian,p));
              break;
            }
            case EXIF_FMT_SLONG:
            {
              EXIFMultipleValues("%.20g",(double)
                ReadPropertySignedLong(endian,p));
              break;
            }
            case EXIF_FMT_URATIONAL:
            {
              if ((tag_value == GPS_LATITUDE) || (tag_value == GPS_LONGITUDE) ||
                  (tag_value == GPS_TIMESTAMP))
                {
                  components=1;
                  EXIFGPSFractions("%.20g/%.20g,%.20g/%.20g,%.20g/%.20g",
                    (double) ReadPropertyUnsignedLong(endian,p),
                    (double) ReadPropertyUnsignedLong(endian,p+4),
                    (double) ReadPropertyUnsignedLong(endian,p+8),
                    (double) ReadPropertyUnsignedLong(endian,p+12),
                    (double) ReadPropertyUnsignedLong(endian,p+16),
                    (double) ReadPropertyUnsignedLong(endian,p+20));
                  break;
                }
              EXIFMultipleFractions("%.20g/%.20g",(double)
                ReadPropertyUnsignedLong(endian,p),(double)
                ReadPropertyUnsignedLong(endian,p+4));
              break;
            }
            case EXIF_FMT_SRATIONAL:
            {
              EXIFMultipleFractions("%.20g/%.20g",(double)
                ReadPropertySignedLong(endian,p),(double)
                ReadPropertySignedLong(endian,p+4));
              break;
            }
            case EXIF_FMT_SINGLE:
            {
              EXIFMultipleValues("%.20g",(double)
                ReadPropertySignedLong(endian,p));
              break;
            }
            case EXIF_FMT_DOUBLE:
            {
              EXIFMultipleValues("%.20g",(double)
                ReadPropertySignedLong(endian,p));
              break;
            }
            case EXIF_FMT_STRING:
            case EXIF_FMT_UNDEFINED:
            default:
            {
              if ((p < exif) || (p > (exif+length-number_bytes)))
                break;
              value=(char *) NULL;
              if (~((size_t) number_bytes) >= 1)
                value=(char *) AcquireQuantumMemory((size_t) number_bytes+1UL,
                  sizeof(*value));
              if (value != (char *) NULL)
                {
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

              key=AcquireString(property);
              switch (all)
              {
                case 1:
                {
                  const char
                    *description;

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
                  if (depth == 2)
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
                  if (depth == 2)
                    (void) SubstituteString(&key,"exif:","exif:thumbnail:");
                }
              }
              if ((image->properties == (void *) NULL) ||
                  (GetValueFromSplayTree((SplayTreeInfo *) image->properties,key) == (const void *) NULL))
                (void) SetImageProperty((Image *) image,key,value,exception);
              value=DestroyString(value);
              key=DestroyString(key);
            }
        }