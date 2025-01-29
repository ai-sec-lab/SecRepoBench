count=FormatLocaleString(header,MagickPathExtent,"#%.*s\n",
        MagickPathExtent-3,property);
      (void) WriteBlob(img,(size_t) count,(unsigned char *) header);