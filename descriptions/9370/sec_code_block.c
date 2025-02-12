      case TIFF_SHORT:
      {
        if (exif_info[i].variable_length == 0)
          {
            uint16
              shorty;

            shorty=0;
            if (TIFFGetField(tiff,exif_info[i].tag,&shorty,sans) == 1)
              (void) FormatLocaleString(value,MagickPathExtent,"%d",shorty);
          }
        else if (exif_info[i].variable_length == 2)
        {
          uint16
            *shorty;
 
          shorty=0;
          if ((TIFFGetField(tiff,exif_info[i].tag,&shorty,sans) == 1) &&
              (shorty != (uint16 *) NULL))
            (void) FormatLocaleString(value,MagickPathExtent,"%d",*shorty);
        }
        else
          {
            int
              tiff_status;

            uint16
              *shorty;

            uint16
              shorty_num;

            tiff_status=TIFFGetField(tiff,exif_info[i].tag,&shorty_num,&shorty,
              sans);
            if (tiff_status == 1)
              (void) FormatLocaleString(value,MagickPathExtent,"%d",
                shorty_num != 0 ? shorty[0] : 0);
          }
        break;
      }