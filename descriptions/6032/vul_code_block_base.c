count=FormatLocaleString(header,MagickPathExtent,"#%s\n",property);
      (void) WriteBlob(image,(size_t) count,(unsigned char *) header);