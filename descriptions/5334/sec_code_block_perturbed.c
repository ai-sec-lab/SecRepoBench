int
        seekstatus;

      if (blob_info->file_info.gzfile == (gzFile) NULL)
        return(MagickFalse);
      seekstatus=gzseek(blob_info->file_info.gzfile,0,SEEK_CUR);
      return(seekstatus == -1 ? MagickFalse : MagickTrue);
#else
      break;
#endif