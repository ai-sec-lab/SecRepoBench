static int read_user_chunk_callback(png_struct *pngptr, png_unknown_chunkp chunk)
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
       chunk->name[0],chunk->name[1],chunk->name[2],chunk->name[3]);

  if (chunk->name[0]  == 101 &&
      (chunk->name[1] ==  88 || chunk->name[1] == 120 ) &&
      chunk->name[2] ==   73 &&
      chunk-> name[3] == 102)
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

      image=(Image *) png_get_user_chunk_ptr(pngptr);

      error_info=(PNGErrorInfo *) png_get_error_ptr(pngptr);

      profile=BlobToStringInfo((const void *) NULL,chunk->size+6);

      if (profile == (StringInfo *) NULL)
        {
          (void) ThrowMagickException(error_info->exception,GetMagickModule(),
            ResourceLimitError,"MemoryAllocationFailed","`%s'",
            image->filename);
          return(-1);
        }
      p=GetStringInfoDatum(profile);

      /* Initialize profile with "Exif\0\0" */
      *p++ ='E';
      *p++ ='x';
      *p++ ='i';
      *p++ ='f';
      *p++ ='\0';
      *p++ ='\0';

      s=chunk->data;
      i=0;
      // <MASK>
    }

  /* orNT */
  if (chunk->name[0] == 111 &&
      chunk->name[1] == 114 &&
      chunk->name[2] ==  78 &&
      chunk->name[3] ==  84)
    {
     /* recognized orNT */
     if (chunk->size != 1)
       return(-1); /* Error return */

     image=(Image *) png_get_user_chunk_ptr(pngptr);

     image->orientation=
       Magick_Orientation_from_Exif_Orientation((int) chunk->data[0]);

     return(1);
    }

  /* vpAg (deprecated, replaced by caNv) */
  if (chunk->name[0] == 118 &&
      chunk->name[1] == 112 &&
      chunk->name[2] ==  65 &&
      chunk->name[3] == 103)
    {
      /* recognized vpAg */

      if (chunk->size != 9)
        return(-1); /* Error return */

      if (chunk->data[8] != 0)
        return(0);  /* ImageMagick requires pixel units */

      image=(Image *) png_get_user_chunk_ptr(pngptr);

      image->page.width=(size_t)mng_get_long(chunk->data);
      image->page.height=(size_t)mng_get_long(&chunk->data[4]);

      return(1);
    }

  /* caNv */
  if (chunk->name[0] ==  99 &&
      chunk->name[1] ==  97 &&
      chunk->name[2] ==  78 &&
      chunk->name[3] == 118)
    {
      /* recognized caNv */

      if (chunk->size != 16)
        return(-1); /* Error return */

      image=(Image *) png_get_user_chunk_ptr(pngptr);

      image->page.width=(size_t)mng_get_long(chunk->data);
      image->page.height=(size_t)mng_get_long(&chunk->data[4]);
      image->page.x=(size_t)mng_get_long(&chunk->data[8]);
      image->page.y=(size_t)mng_get_long(&chunk->data[12]);

      return(1);
    }

  return(0); /* Did not recognize */
}