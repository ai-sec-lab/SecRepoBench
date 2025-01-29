if ((TIFFGetField(tiff,TIFFTAG_ARTIST,&text) == 1) &&
      (text != (char *) NULL))
    (void) SetImageProperty(img,"tiff:artist",text,exception);
  if ((TIFFGetField(tiff,TIFFTAG_COPYRIGHT,&text) == 1) &&
      (text != (char *) NULL))
    (void) SetImageProperty(img,"tiff:copyright",text,exception);
  if ((TIFFGetField(tiff,TIFFTAG_DATETIME,&text) == 1) &&
      (text != (char *) NULL))
    (void) SetImageProperty(img,"tiff:timestamp",text,exception);
  if ((TIFFGetField(tiff,TIFFTAG_DOCUMENTNAME,&text) == 1) &&
      (text != (char *) NULL))
    (void) SetImageProperty(img,"tiff:document",text,exception);
  if ((TIFFGetField(tiff,TIFFTAG_HOSTCOMPUTER,&text) == 1) &&
      (text != (char *) NULL))
    (void) SetImageProperty(img,"tiff:hostcomputer",text,exception);
  if ((TIFFGetField(tiff,TIFFTAG_IMAGEDESCRIPTION,&text) == 1) &&
      (text != (char *) NULL))
    (void) SetImageProperty(img,"comment",text,exception);
  if ((TIFFGetField(tiff,TIFFTAG_MAKE,&text) == 1) &&
      (text != (char *) NULL))
    (void) SetImageProperty(img,"tiff:make",text,exception);
  if ((TIFFGetField(tiff,TIFFTAG_MODEL,&text) == 1) &&
      (text != (char *) NULL))
    (void) SetImageProperty(img,"tiff:model",text,exception);
  if ((TIFFGetField(tiff,TIFFTAG_OPIIMAGEID,&count,&text) == 1) &&
      (text != (char *) NULL))
    {
      if (count >= MagickPathExtent)
        count=MagickPathExtent-1;
      (void) CopyMagickString(message,text,count+1);
      (void) SetImageProperty(img,"tiff:image-id",message,exception);
    }
  if ((TIFFGetField(tiff,TIFFTAG_PAGENAME,&text) == 1) &&
      (text != (char *) NULL))
    (void) SetImageProperty(img,"label",text,exception);
  if ((TIFFGetField(tiff,TIFFTAG_SOFTWARE,&text) == 1) &&
      (text != (char *) NULL))
    (void) SetImageProperty(img,"tiff:software",text,exception);
  if ((TIFFGetField(tiff,33423,&count,&text) == 1) && (text != (char *) NULL))
    {
      if (count >= MagickPathExtent)
        count=MagickPathExtent-1;
      (void) CopyMagickString(message,text,count+1);
      (void) SetImageProperty(img,"tiff:kodak-33423",message,exception);
    }
  if ((TIFFGetField(tiff,36867,&count,&text) == 1) && (text != (char *) NULL))
    {
      if (count >= MagickPathExtent)
        count=MagickPathExtent-1;
      (void) CopyMagickString(message,text,count+1);
      (void) SetImageProperty(img,"tiff:kodak-36867",message,exception);
    }
  if (TIFFGetField(tiff,TIFFTAG_SUBFILETYPE,&type) == 1)
    switch (type)
    {
      case 0x01:
      {
        (void) SetImageProperty(img,"tiff:subfiletype","REDUCEDIMAGE",
          exception);
        break;
      }
      case 0x02:
      {
        (void) SetImageProperty(img,"tiff:subfiletype","PAGE",exception);
        break;
      }
      case 0x04:
      {
        (void) SetImageProperty(img,"tiff:subfiletype","MASK",exception);
        break;
      }
      default:
        break;
    }