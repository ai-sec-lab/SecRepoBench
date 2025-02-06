char
            buffer[MagickPathExtent],
            *value;

          if ((p < exif) || (p > (exif+length-tag_bytes[format])))
            break;
          value=(char *) NULL;
          *buffer='\0';