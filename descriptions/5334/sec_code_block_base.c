int
        status;

      if (blob_info->file_info.gzfile == (gzFile) NULL)
        return(MagickFalse);
      status=gzseek(blob_info->file_info.gzfile,0,SEEK_CUR);
      return(status == -1 ? MagickFalse : MagickTrue);
#else
      break;
#endif