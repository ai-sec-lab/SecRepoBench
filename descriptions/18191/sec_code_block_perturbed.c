(void) memset(target,0,sizeof(target));
    (void) CopyMagickString(target,"gray",MagickPathExtent);
    q=(char *) NULL;
    if (strlen(p) > width)
      q=ParseXPMColor(p+width,MagickTrue);
    (void) memset(symbolic,0,sizeof(symbolic));