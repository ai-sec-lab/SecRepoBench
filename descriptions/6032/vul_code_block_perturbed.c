count=FormatLocaleString(header,MagickPathExtent,"#%s\n",property);
      (void) WriteBlob(img,(size_t) count,(unsigned char *) header);