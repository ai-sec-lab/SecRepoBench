static Image *ReadORAImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  char
    image_data_buffer[8192];

  const char
    *MERGED_IMAGE_PATH = "mergedimage.png";

   FILE
    *file;

  Image
    *image_metadata,
    *out_image;

  ImageInfo
    *read_info;

  int
    unique_file,
    zip_error;

  MagickBooleanType
    status;

  struct stat
    stat_info;

  zip_t
    *zip_archive;

  zip_file_t
    *merged_image_file;

  zip_int64_t
    read_bytes;

  zip_uint64_t
    offset;

  image_metadata=AcquireImage(image_info,exception);
  read_info=CloneImageInfo(image_info);
  SetImageInfoBlob(read_info,(void *) NULL,0);
  (void) stat(image_info->filename,&stat_info);
  zip_archive=zip_open(image_info->filename,ZIP_RDONLY,&zip_error);
  if (zip_archive == NULL)
    {
      ThrowFileException(exception,FileOpenError,"UnableToOpenFile",
        image_info->filename);
      read_info=DestroyImageInfo(read_info);
      image_metadata=DestroyImage(image_metadata);
      return((Image *) NULL);
    }
  merged_image_file=zip_fopen(zip_archive,MERGED_IMAGE_PATH,ZIP_FL_UNCHANGED);
  if (merged_image_file == NULL)
    {
      ThrowFileException(exception,FileOpenError,"UnableToOpenFile",
        image_info->filename);
      read_info=DestroyImageInfo(read_info);
      image_metadata=DestroyImage(image_metadata);
      zip_discard(zip_archive);
      return((Image *) NULL);
    }
  /* Get a temporary file to write the mergedimage.png of the ZIP to */
  (void) CopyMagickString(read_info->magick,"PNG",MagickPathExtent);
  unique_file=AcquireUniqueFileResource(read_info->unique);
  (void) CopyMagickString(read_info->filename,read_info->unique,
    MagickPathExtent);
  file=(FILE *) NULL;
  if (unique_file != -1)
    file=fdopen(unique_file,"wb");
  if ((unique_file == -1) || (file == (FILE *) NULL))
    {
      ThrowFileException(exception,FileOpenError,"UnableToCreateTemporaryFile",
        read_info->filename);
      if (unique_file != -1)
        (void) RelinquishUniqueFileResource(read_info->filename);
      read_info=DestroyImageInfo(read_info);
      image_metadata=DestroyImage(image_metadata);
      zip_fclose(merged_image_file);
      zip_discard(zip_archive);
      return((Image *) NULL);
    }
  /* Write the uncompressed mergedimage.png to the temporary file */
  status=MagickTrue;
  offset=0;
  while (status != MagickFalse)
  {
    read_bytes=zip_fread(merged_image_file,image_data_buffer+offset,
      sizeof(image_data_buffer)-offset);
    if (read_bytes == -1)
      status=MagickFalse;
    else if (read_bytes == 0)
      {
        /* Write up to offset of image_data_buffer to temp file */
        if (!fwrite(image_data_buffer,offset,1,file))
          status=MagickFalse;
        break;
      }
    else if (read_bytes == (ssize_t) (sizeof(image_data_buffer)-offset))
      {
        /* Write the entirely of image_data_buffer to temp file */
        if (!fwrite(image_data_buffer,sizeof(image_data_buffer),1,file))
          status=MagickFalse;
        else
          offset=0;
      }
    else
      offset+=(zip_int64_t) read_bytes;
  }
  (void) fclose(file);
  (void) zip_fclose(merged_image_file);
  (void) zip_discard(zip_archive);
  if (status == MagickFalse)
    {
      ThrowFileException(exception,CoderError,"UnableToReadImageData",
          read_info->filename);
      (void) RelinquishUniqueFileResource(read_info->filename);
      read_info=DestroyImageInfo(read_info);
      image_metadata=DestroyImage(image_metadata);
      return((Image *) NULL);
    }
  /* Delegate to ReadImage to read mergedimage.png */
  out_image=ReadImage(read_info,exception);
  (void) RelinquishUniqueFileResource(read_info->filename);
  read_info=DestroyImageInfo(read_info);
  /* Update fields of image from fields of png_image */
  if (out_image != NULL)
    {
      (void) CopyMagickString(out_image->filename,image_metadata->filename,
        MagickPathExtent);
      (void) CopyMagickString(out_image->magick_filename,
        image_metadata->magick_filename,MagickPathExtent);
      out_image->timestamp=time(&stat_info.st_mtime);
      (void) CopyMagickString(out_image->magick,image_metadata->magick,
        MagickPathExtent);
      out_image->extent=(MagickSizeType) stat_info.st_size;
    }
  image_metadata=DestroyImage(image_metadata);
  return(out_image);
}