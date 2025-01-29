for (i=0; i < (ssize_t) image->colors; i++)
    {
      image->colormap[i].red=ScaleCharToQuantum(*p++);
      image->colormap[i].green=ScaleCharToQuantum(*p++);
      image->colormap[i].blue=ScaleCharToQuantum(*p++);
    }
    (void) SetImageBackgroundColor(image,exception);
    pcxheader.bytes_per_line=ReadBlobLSBShort(image);
    pcxheader.palette_info=ReadBlobLSBShort(image);
    pcxheader.horizontal_screensize=ReadBlobLSBShort(image);