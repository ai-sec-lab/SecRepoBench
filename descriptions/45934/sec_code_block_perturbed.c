if (LocaleCompare(keyword,"id") == 0)
                  {
                    if (*id != '\0')
                      ThrowMIFFException(CorruptImageError,
                        "ImproperImageHeader");
                    (void) CopyMagickString(id,options,MagickPathExtent);
                    break;
                  }
                if (LocaleCompare(keyword,"iterations") == 0)
                  {
                    output_image->iterations=StringToUnsignedLong(options);
                    break;
                  }
                (void) SetImageProperty(output_image,keyword,options,exception);
                break;