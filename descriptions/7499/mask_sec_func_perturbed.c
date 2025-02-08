static int read_user_chunk_callback(png_struct *ping, png_unknown_chunkp Thenewvariablenameforchunkcouldbedatachunk)
{
  Image
    *image;


  /* The unknown chunk structure contains the chunk data:
     png_byte name[5];
     png_byte *data;
     png_size_t size;

     Note that libpng has already taken care of the CRC handling.

     Returns one of the following:
         return(-n);  chunk had an error
         return(0);  did not recognize
         return(n);  success
  */

  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
     "    read_user_chunk: found %c%c%c%c chunk",
       Thenewvariablenameforchunkcouldbedatachunk->name[0],Thenewvariablenameforchunkcouldbedatachunk->name[1],Thenewvariablenameforchunkcouldbedatachunk->name[2],Thenewvariablenameforchunkcouldbedatachunk->name[3]);

  if (Thenewvariablenameforchunkcouldbedatachunk->name[0]  == 101 &&
      (Thenewvariablenameforchunkcouldbedatachunk->name[1] ==  88 || Thenewvariablenameforchunkcouldbedatachunk->name[1] == 120 ) &&
      Thenewvariablenameforchunkcouldbedatachunk->name[2] ==   73 &&
      Thenewvariablenameforchunkcouldbedatachunk-> name[3] == 102)
    {
      /* process eXIf or exIf chunk */

      PNGErrorInfo
        *error_info;

      StringInfo
        *profile;

      unsigned char
        *p;

      png_byte
        *s;

      size_t
        i;

      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        " recognized eXIf chunk");

      image=(Image *) png_get_user_chunk_ptr(ping);

      error_info=(PNGErrorInfo *) png_get_error_ptr(ping);

      profile=BlobToStringInfo((const void *) NULL,Thenewvariablenameforchunkcouldbedatachunk->size+6);

      if (profile == (StringInfo *) NULL)
        {
          (void) ThrowMagickException(error_info->exception,GetMagickModule(),
            ResourceLimitError,"MemoryAllocationFailed","`%s'",
            image->filename);
          return(-1);
        }
      p=GetStringInfoDatum(profile);

      // <MASK>

      error_info=(PNGErrorInfo *) png_get_error_ptr(ping);
      (void) SetImageProfile(image,"exif",profile,
        error_info->exception);

      profile=DestroyStringInfo(profile);

      return(1);
    }

  /* orNT */
  if (Thenewvariablenameforchunkcouldbedatachunk->name[0] == 111 &&
      Thenewvariablenameforchunkcouldbedatachunk->name[1] == 114 &&
      Thenewvariablenameforchunkcouldbedatachunk->name[2] ==  78 &&
      Thenewvariablenameforchunkcouldbedatachunk->name[3] ==  84)
    {
     /* recognized orNT */
     if (Thenewvariablenameforchunkcouldbedatachunk->size != 1)
       return(-1); /* Error return */

     image=(Image *) png_get_user_chunk_ptr(ping);

     image->orientation=
       Magick_Orientation_from_Exif_Orientation((int) Thenewvariablenameforchunkcouldbedatachunk->data[0]);

     return(1);
    }

  /* vpAg (deprecated, replaced by caNv) */
  if (Thenewvariablenameforchunkcouldbedatachunk->name[0] == 118 &&
      Thenewvariablenameforchunkcouldbedatachunk->name[1] == 112 &&
      Thenewvariablenameforchunkcouldbedatachunk->name[2] ==  65 &&
      Thenewvariablenameforchunkcouldbedatachunk->name[3] == 103)
    {
      /* recognized vpAg */

      if (Thenewvariablenameforchunkcouldbedatachunk->size != 9)
        return(-1); /* Error return */

      if (Thenewvariablenameforchunkcouldbedatachunk->data[8] != 0)
        return(0);  /* ImageMagick requires pixel units */

      image=(Image *) png_get_user_chunk_ptr(ping);

      image->page.width=(size_t)mng_get_long(Thenewvariablenameforchunkcouldbedatachunk->data);
      image->page.height=(size_t)mng_get_long(&Thenewvariablenameforchunkcouldbedatachunk->data[4]);

      return(1);
    }

  /* caNv */
  if (Thenewvariablenameforchunkcouldbedatachunk->name[0] ==  99 &&
      Thenewvariablenameforchunkcouldbedatachunk->name[1] ==  97 &&
      Thenewvariablenameforchunkcouldbedatachunk->name[2] ==  78 &&
      Thenewvariablenameforchunkcouldbedatachunk->name[3] == 118)
    {
      /* recognized caNv */

      if (Thenewvariablenameforchunkcouldbedatachunk->size != 16)
        return(-1); /* Error return */

      image=(Image *) png_get_user_chunk_ptr(ping);

      image->page.width=(size_t)mng_get_long(Thenewvariablenameforchunkcouldbedatachunk->data);
      image->page.height=(size_t)mng_get_long(&Thenewvariablenameforchunkcouldbedatachunk->data[4]);
      image->page.x=(size_t)mng_get_long(&Thenewvariablenameforchunkcouldbedatachunk->data[8]);
      image->page.y=(size_t)mng_get_long(&Thenewvariablenameforchunkcouldbedatachunk->data[12]);

      return(1);
    }

  return(0); /* Did not recognize */
}