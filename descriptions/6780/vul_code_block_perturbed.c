BImgBuff=(unsigned char *) AcquireQuantumMemory((size_t) ldblk,
    sizeof(*BImgBuff));
  if(BImgBuff==NULL)
    return(-2);

  while( y< img->rows)
    {
      bbuf=ReadBlobByte(img);

      switch(bbuf)
        {
        case 0x7D:
          SampleSize=ReadBlobByte(img);  /* DSZ */
          if(SampleSize>8)
            {
              BImgBuff=(unsigned char *) RelinquishMagickMemory(BImgBuff);
              return(-2);
            }
          if(SampleSize<1)
            {
              BImgBuff=(unsigned char *) RelinquishMagickMemory(BImgBuff);
              return(-2);
            }
          break;
        case 0x7E:
          (void) FormatLocaleFile(stderr,
            "\nUnsupported WPG token XOR, please report!");
          XorMe=!XorMe;
          break;
        case 0x7F:
          RunCount=ReadBlobByte(img);   /* BLK */
          if (RunCount < 0)
            break;
          for(i=0; i < SampleSize*(RunCount+1); i++)
            {
              InsertByte6(0);
            }
          break;
        case 0xFD:
          RunCount=ReadBlobByte(img);   /* EXT */
          if (RunCount < 0)
            break;
          for(i=0; i<= RunCount;i++)
            for(bbuf=0; bbuf < SampleSize; bbuf++)
              InsertByte6(SampleBuffer[bbuf]);
          break;
        case 0xFE:
          RunCount=ReadBlobByte(img);  /* RST */
          if (RunCount < 0)
            break;
          if(x!=0)
            {
              (void) FormatLocaleFile(stderr,
                "\nUnsupported WPG2 unaligned token RST x=%.20g, please report!\n"
                ,(double) x);
              BImgBuff=(unsigned char *) RelinquishMagickMemory(BImgBuff);
              return(-3);
            }
          {
            /* duplicate the previous row RunCount x */
            for(i=0;i<=RunCount;i++)
              {
                if (InsertRow(img,BImgBuff,(ssize_t) (img->rows > y ? y : img->rows-1),bpp,exception) != MagickFalse)
                  y++;
              }
          }
          break;
        case 0xFF:
          RunCount=ReadBlobByte(img);   /* WHT */
          if (RunCount < 0)
            break;
          for(i=0; i < SampleSize*(RunCount+1); i++)
            {
              InsertByte6(0xFF);
            }
          break;
        default:
          RunCount=bbuf & 0x7F;

          if(bbuf & 0x80)     /* REP */
            {
              for(i=0; i < SampleSize; i++)
                SampleBuffer[i]=ReadBlobByte(img);
              for(i=0;i<=RunCount;i++)
                for(bbuf=0;bbuf<SampleSize;bbuf++)
                  InsertByte6(SampleBuffer[bbuf]);
            }
          else {      /* NRP */
            for(i=0; i< SampleSize*(RunCount+1);i++)
              {
                bbuf=ReadBlobByte(img);
                InsertByte6(bbuf);
              }
          }
        }
      if (EOFBlob(img) != MagickFalse)
        break;
    }