static Image *ReadSCRImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
    char zxscr[6144];
    char zxattr[768];
    int octetnr;
    int octetline;
    int zoneline;
    int zonenr;
    int octet_val;
    int attr_nr;
    int pix;
    int piy;
    int binar[8];
    int attrbin[8];
    int *pbin;
    int *abin;
    int z;
    int one_nr;
    int ink;
    int paper;
    int intintensityflag;

  unsigned char colour_palette[] = {
      0,  0,  0,
      0,  0,192,
    192,  0,  0,
    192,  0,192,
      0,192,  0,
      0,192,192,
    192,192,  0,
    192,192,192,
      0,  0,  0,
      0,  0,255,
    255,  0,  0,
    255,  0,255,
      0,255,  0,
      0,255,255,
    255,255,  0,
    255,255,255
  };

  Image
    *image;

  MagickBooleanType
    status;

  register Quantum
    *q;

  ssize_t
    count;

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
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  image->columns = 256;
  image->rows = 192;
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  count=ReadBlob(image,6144,(unsigned char *) zxscr);
  if (count != 6144)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  count=ReadBlob(image,768,(unsigned char *) zxattr);
  if (count != 768)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  for(zonenr=0;zonenr<3;zonenr++)
  {
      for(zoneline=0;zoneline<8;zoneline++)
        {
        for(octetline=0;octetline<8;octetline++)
      {
          for(octetnr=(zoneline*32);octetnr<((zoneline*32)+32);octetnr++)
            {
            octet_val = zxscr[octetnr+(256*octetline)+(zonenr*2048)];
            attr_nr = zxattr[octetnr+(256*zonenr)];

            pix = (((8*octetnr)-(256*zoneline)));
            piy = ((octetline+(8*zoneline)+(zonenr*64)));

            pbin = binar;
            abin = attrbin;

            one_nr=1;

            for(z=0;z<8;z++)
          {
              if(octet_val&one_nr)
            {
                *pbin = 1;
            } else {
                *pbin = 0;
            }
              one_nr=one_nr*2;
              pbin++;
          }

            one_nr = 1;

            for(z=0;z<8;z++)
          {
              if(attr_nr&one_nr)
            {
                *abin = 1;
            } else {
                *abin = 0;
            }
              one_nr=one_nr*2;
              abin++;
          }

            ink = (attrbin[0]+(2*attrbin[1])+(4*attrbin[2]));
            paper = (attrbin[3]+(2*attrbin[4])+(4*attrbin[5]));
            intintensityflag = attrbin[6];

            if(intintensityflag) { ink=ink+8; paper=paper+8; }

            for(z=7;z>-1;z--)
          {
              q=QueueAuthenticPixels(image,pix,piy,1,1,exception);
              if (q == (Quantum *) NULL)
                break;

              if(binar[z])
            {
                SetPixelRed(image,ScaleCharToQuantum(
                  colour_palette[3*ink]),q);
                SetPixelGreen(image,ScaleCharToQuantum(
                  colour_palette[1+(3*ink)]),q);
                SetPixelBlue(image,ScaleCharToQuantum(
                  colour_palette[2+(3*ink)]),q);
            } else {
                SetPixelRed(image,ScaleCharToQuantum(
                  colour_palette[3*paper]),q);
                SetPixelGreen(image,ScaleCharToQuantum(
                  colour_palette[1+(3*paper)]),q);
                SetPixelBlue(image,ScaleCharToQuantum(
                  colour_palette[2+(3*paper)]),q);
            }

              pix++;
          }
        }
      }
    }
  }
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}