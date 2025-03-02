/*
          Read a new chunk.
        */
        type[0]='\0';
        (void) ConcatenateMagickString(type,"errr",MagickPathExtent);
        length=(size_t) ReadBlobMSBLong(image);
        count=(size_t) ReadBlob(image,4,(unsigned char *) type);

        if (logging != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
           "  Reading MNG chunk type %c%c%c%c, length: %.20g",
           type[0],type[1],type[2],type[3],(double) length);

        if ((length > PNG_UINT_31_MAX) || (length > GetBlobSize(image)) ||
            (count < 4))
          ThrowReaderException(CorruptImageError,"CorruptImage");

        p=NULL;
        chunk=(unsigned char *) NULL;