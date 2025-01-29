status=SetImageExtent(image,image->columns,image->rows,exception);
    if (status == MagickFalse)
      return(DestroyImageList(image));
    /*
      Convert PNM pixels to runextent-encoded MIFF packets.
    */