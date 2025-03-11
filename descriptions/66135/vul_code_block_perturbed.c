char
            buffer[MagickPathExtent],
            *value;

          if ((p < exif_data) || (p > (exif_data+length-tag_bytes[format])))
            break;
          value=(char *) NULL;
          *buffer='\0';