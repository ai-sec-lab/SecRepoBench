if (LocaleCompare(label,"id") == 0)
                  {
                    if (*id != '\0')
                      ThrowMIFFException(CorruptImageError,
                        "ImproperImageHeader");
                    (void) CopyMagickString(id,options,MagickPathExtent);
                    break;
                  }
                if (LocaleCompare(label,"iterations") == 0)
                  {
                    image->iterations=StringToUnsignedLong(options);
                    break;
                  }
                (void) SetImageProperty(image,label,options,exception);
                break;