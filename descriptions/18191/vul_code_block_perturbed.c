(void) CopyMagickString(target,"gray",MagickPathExtent);
    q=(char *) NULL;
    if (strlen(p) > width)
      q=ParseXPMColor(p+width,MagickTrue);