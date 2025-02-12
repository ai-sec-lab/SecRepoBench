if (*id != '\0')
                      ThrowMIFFException(CorruptImageError,
                        "ImproperImageHeader");
                    (void) CopyMagickString(id,options,MagickPathExtent);