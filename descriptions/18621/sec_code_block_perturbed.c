ssize_t
        row_offset;

      stream=ReadBlobStream(image,extent,pixels,&count);
      if (count != (ssize_t) extent)
        break;
      if ((image->progress_monitor != (MagickProgressMonitor) NULL) &&
          (image->previous == (Image *) NULL))
        {
          MagickBooleanType
            proceed;

          proceed=SetImageProgress(image,LoadImageTag,(MagickOffsetType) r,
            image->rows);
          if (proceed == MagickFalse)
            break;
        }
      row_offset=r++;
      q=QueueAuthenticPixels(image,0,row_offset,image->columns,1,exception);