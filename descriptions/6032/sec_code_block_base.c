count=FormatLocaleString(header,MagickPathExtent,"#%.*s\n",
        MagickPathExtent-3,property);
      (void) WriteBlob(image,(size_t) count,(unsigned char *) header);