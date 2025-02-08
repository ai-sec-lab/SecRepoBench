static MagickBooleanType InsertRow(Image *img,unsigned char *p,ssize_t y,
  int bpp,ExceptionInfo *exception)
{
  int
    bit;

  Quantum
    index,
    *q;

  ssize_t
    x;

  q=QueueAuthenticPixels(img,0,y,img->columns,1,exception);
  if (q == (Quantum *) NULL)
    return(MagickFalse);
  switch (bpp)
    {
    case 1:  /* Convert bitmap scanline. */
      {
        for (x=0; x < ((ssize_t) img->columns-7); x+=8)
        {
          for (bit=0; bit < 8; bit++)
          {
            index=((*p) & (0x80 >> bit) ? 0x01 : 0x00);
            SetPixelIndex(img,index,q);
            if (index < img->colors)
              SetPixelViaPixelInfo(img,img->colormap+(ssize_t) index,q);
            q+=GetPixelChannels(img);
          }
          p++;
        }
        if ((img->columns % 8) != 0)
          {
            for (bit=0; bit < (ssize_t) (img->columns % 8); bit++)
            {
              index=((*p) & (0x80 >> bit) ? 0x01 : 0x00);
              SetPixelIndex(img,index,q);
              if (index < img->colors)
                SetPixelViaPixelInfo(img,img->colormap+(ssize_t) index,q);
              q+=GetPixelChannels(img);
            }
            p++;
          }
        break;
      }
    case 2:  /* Convert PseudoColor scanline. */
      {
        for (x=0; x < ((ssize_t) img->columns-3); x+=4)
        {
            index=ConstrainColormapIndex(img,(*p >> 6) & 0x3,exception);
            SetPixelIndex(img,index,q);
            if (index < img->colors)
              SetPixelViaPixelInfo(img,img->colormap+(ssize_t) index,q);
            q+=GetPixelChannels(img);
            index=ConstrainColormapIndex(img,(*p >> 4) & 0x3,exception);
            SetPixelIndex(img,index,q);
            if (index < img->colors)
              SetPixelViaPixelInfo(img,img->colormap+(ssize_t) index,q);
            q+=GetPixelChannels(img);
            index=ConstrainColormapIndex(img,(*p >> 2) & 0x3,exception);
            SetPixelIndex(img,index,q);
            if (index < img->colors)
              SetPixelViaPixelInfo(img,img->colormap+(ssize_t) index,q);
            q+=GetPixelChannels(img);
            index=ConstrainColormapIndex(img,(*p) & 0x3,exception);
            SetPixelIndex(img,index,q);
            if (index < img->colors)
              SetPixelViaPixelInfo(img,img->colormap+(ssize_t) index,q);
            q+=GetPixelChannels(img);
            p++;
        }
       if ((img->columns % 4) != 0)
          {
            index=ConstrainColormapIndex(img,(*p >> 6) & 0x3,exception);
            SetPixelIndex(img,index,q);
            if (index < img->colors)
              SetPixelViaPixelInfo(img,img->colormap+(ssize_t) index,q);
            q+=GetPixelChannels(img);
            if ((img->columns % 4) > 1)
              {
                index=ConstrainColormapIndex(img,(*p >> 4) & 0x3,exception);
                SetPixelIndex(img,index,q);
                if (index < img->colors)
                  SetPixelViaPixelInfo(img,img->colormap+(ssize_t) index,q);
                q+=GetPixelChannels(img);
                if ((img->columns % 4) > 2)
                  {
                    index=ConstrainColormapIndex(img,(*p >> 2) & 0x3,
                      exception);
                    SetPixelIndex(img,index,q);
                    if (index < img->colors)
                      SetPixelViaPixelInfo(img,img->colormap+(ssize_t)
                        index,q);
                    q+=GetPixelChannels(img);
                  }
              }
            p++;
          }
        break;
      }

    case 4:  /* Convert PseudoColor scanline. */
      {
        for (x=0; x < ((ssize_t) img->columns-1); x+=2)
          {
            index=ConstrainColormapIndex(img,(*p >> 4) & 0x0f,exception);
            SetPixelIndex(img,index,q);
            if (index < img->colors)
              SetPixelViaPixelInfo(img,img->colormap+(ssize_t) index,q);
            q+=GetPixelChannels(img);
            index=ConstrainColormapIndex(img,(*p) & 0x0f,exception);
            SetPixelIndex(img,index,q);
            if (index < img->colors)
              SetPixelViaPixelInfo(img,img->colormap+(ssize_t) index,q);
            p++;
            q+=GetPixelChannels(img);
          }
        if ((img->columns % 2) != 0)
          {
            index=ConstrainColormapIndex(img,(*p >> 4) & 0x0f,exception);
            SetPixelIndex(img,index,q);
            if (index < img->colors)
              SetPixelViaPixelInfo(img,img->colormap+(ssize_t) index,q);
            p++;
            q+=GetPixelChannels(img);
          }
        break;
      }
    case 8: /* Convert PseudoColor scanline. */
      {
        for (x=0; x < (ssize_t) img->columns; x++)
          {
            index=ConstrainColormapIndex(img,*p,exception);
            SetPixelIndex(img,index,q);
            if (index < img->colors)
              SetPixelViaPixelInfo(img,img->colormap+(ssize_t) index,q);
            p++;
            q+=GetPixelChannels(img);
          }
      }
      break;

    case 24:     /*  Convert DirectColor scanline.  */
      for (x=0; x < (ssize_t) img->columns; x++)
        {
          SetPixelRed(img,ScaleCharToQuantum(*p++),q);
          SetPixelGreen(img,ScaleCharToQuantum(*p++),q);
          SetPixelBlue(img,ScaleCharToQuantum(*p++),q);
          q+=GetPixelChannels(img);
        }
      break;
    default:
      return(MagickFalse);
    }
  if (!SyncAuthenticPixels(img,exception))
    return(MagickFalse);
  return(MagickTrue);
}