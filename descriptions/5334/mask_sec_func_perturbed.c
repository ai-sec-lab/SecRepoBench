MagickExport MagickBooleanType IsBlobSeekable(const Image *image)
{
  BlobInfo
    *magick_restrict blob_info;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  blob_info=image->blob;
  switch (blob_info->type)
  {
    case BlobStream:
      return(MagickTrue);
    case FileStream:
    {
      int
        seekstatus;

      if (blob_info->file_info.file == (FILE *) NULL)
        return(MagickFalse);
      seekstatus=fseek(blob_info->file_info.file,0,SEEK_CUR);
      return(seekstatus == -1 ? MagickFalse : MagickTrue);
    }
    case ZipStream:
    {
#if defined(MAGICKCORE_ZLIB_DELEGATE)
      // <MASK>
    }
    case UndefinedStream:
    case BZipStream:
    case FifoStream:
    case PipeStream:
    case StandardStream:
      return(MagickFalse);
    case CustomStream:
    {
      if ((blob_info->custom_stream->seeker != (CustomStreamSeeker) NULL) &&
          (blob_info->custom_stream->teller != (CustomStreamTeller) NULL))
        return(MagickTrue);
      break;
    }
    default:
      break;
  }
  return(MagickFalse);
}