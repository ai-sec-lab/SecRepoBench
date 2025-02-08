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
    bytesread;

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
  // <MASK>
  image_metadata=DestroyImage(image_metadata);
  return(out_image);
}