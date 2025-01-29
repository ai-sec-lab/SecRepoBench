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
      q=QueueAuthenticPixels(image,0,++r,image->columns,1,exception);