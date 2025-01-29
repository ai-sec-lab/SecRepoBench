(void) SetImageColorspace(image,(ColorspaceType) type,exception);
    (void) SetImageBackgroundColor(image,exception);
    GetPixelInfo(image,&pixel);
    quantumrange=GetQuantumRange(image->depth);
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      double
        alpha,
        black,
        blue,
        green,
        red;

      red=0.0;
      green=0.0;
      blue=0.0;
      black=0.0;
      alpha=0.0;
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        if (ReadBlobString(image,text) == (char *) NULL)
          break;
        switch (image->colorspace)
        {
          case LinearGRAYColorspace:
          case GRAYColorspace:
          {
            if (image->alpha_trait != UndefinedPixelTrait)
              {
                count=(ssize_t) sscanf(text,"%ld,%ld: (%lf%*[%,]%lf%*[%,]",
                  &x_offset,&y_offset,&red,&alpha);
                green=red;
                blue=red;
                break;
              }
            count=(ssize_t) sscanf(text,"%ld,%ld: (%lf%*[%,]",&x_offset,
              &y_offset,&red);
            green=red;
            blue=red;
            break;       
          }
          case CMYKColorspace:
          {
            if (image->alpha_trait != UndefinedPixelTrait)
              {
                count=(ssize_t) sscanf(text,
                  "%ld,%ld: (%lf%*[%,]%lf%*[%,]%lf%*[%,]%lf%*[%,]%lf%*[%,]",
                  &x_offset,&y_offset,&red,&green,&blue,&black,&alpha);
                break;
              }
            count=(ssize_t) sscanf(text,
              "%ld,%ld: (%lf%*[%,]%lf%*[%,]%lf%*[%,]%lf%*[%,]",&x_offset,
              &y_offset,&red,&green,&blue,&black);
            break;
          }
          default:
          {
            if (image->alpha_trait != UndefinedPixelTrait)
              {
                count=(ssize_t) sscanf(text,
                  "%ld,%ld: (%lf%*[%,]%lf%*[%,]%lf%*[%,]%lf%*[%,]",
                  &x_offset,&y_offset,&red,&green,&blue,&alpha);
                break;
              }
            count=(ssize_t) sscanf(text,
              "%ld,%ld: (%lf%*[%,]%lf%*[%,]%lf%*[%,]",&x_offset,
              &y_offset,&red,&green,&blue);
            break;       
          }
        }
        if (strchr(text,'%') != (char *) NULL)
          {
            red*=0.01*quantumrange;
            green*=0.01*quantumrange;
            blue*=0.01*quantumrange;
            black*=0.01*quantumrange;
            alpha*=0.01*quantumrange;
          }
        if (image->colorspace == LabColorspace)
          {
            green+=(quantumrange+1)/2.0;
            blue+=(quantumrange+1)/2.0;
          }
        pixel.red=(MagickRealType) ScaleAnyToQuantum((QuantumAny) (red+0.5),
          quantumrange);
        pixel.green=(MagickRealType) ScaleAnyToQuantum((QuantumAny) (green+0.5),
          quantumrange);
        pixel.blue=(MagickRealType) ScaleAnyToQuantum((QuantumAny) (blue+0.5),
          quantumrange);
        pixel.black=(MagickRealType) ScaleAnyToQuantum((QuantumAny) (black+0.5),
          quantumrange);
        pixel.alpha=(MagickRealType) ScaleAnyToQuantum((QuantumAny) (alpha+0.5),
          quantumrange);
        q=GetAuthenticPixels(image,(ssize_t) x_offset,(ssize_t) y_offset,1,1,
          exception);
        if (q == (Quantum *) NULL)
          continue;
        SetPixelViaPixelInfo(image,&pixel,q);
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
    }
    *text='\0';
    (void) ReadBlobString(image,text);
    if (LocaleNCompare((char *) text,MagickID,strlen(MagickID)) == 0)
      {
        /*
          Allocate next image structure.
        */
        AcquireNextImage(image_info,image,exception);
        if (GetNextImageInList(image) == (Image *) NULL)
          {
            image=DestroyImageList(image);
            return((Image *) NULL);
          }
        image=SyncNextImageInList(image);
        status=SetImageProgress(image,LoadImagesTag,TellBlob(image),
          GetBlobSize(image));
        if (status == MagickFalse)
          break;
      }