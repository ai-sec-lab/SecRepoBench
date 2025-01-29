heif_item_id
    primary_image_id;

  Image
    *image;

  MagickBooleanType
    status;

  size_t
    length;

  struct heif_context
    *heif_context;

  struct heif_error
    error;

  void
    *file_data;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  image=AcquireImage(image_info,exception);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  if (GetBlobSize(image) > (MagickSizeType) SSIZE_MAX)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  length=(size_t) GetBlobSize(image);
  file_data=AcquireMagickMemory(length);
  if (file_data == (void *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  if (ReadBlob(image,length,file_data) != (ssize_t) length)
    {
      file_data=RelinquishMagickMemory(file_data);
      ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");
    }
  /*
    Decode HEIF file
  */
  heif_context=heif_context_alloc();
  error=heif_context_read_from_memory(heif_context,file_data,length,NULL);
  file_data=RelinquishMagickMemory(file_data);
  if (IsHeifSuccess(&error,image,exception) == MagickFalse)
    {
      heif_context_free(heif_context);
      return(DestroyImageList(image));
    }
  error=heif_context_get_primary_image_ID(heif_context,&primary_image_id);
  if (IsHeifSuccess(&error,image,exception) == MagickFalse)
    {
      heif_context_free(heif_context);
      return(DestroyImageList(image));
    }
  status=ReadHEICImageByID(image_info,image,heif_context,primary_image_id,
    exception);
  heif_context_free(heif_context);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  return(GetFirstImageInList(image));