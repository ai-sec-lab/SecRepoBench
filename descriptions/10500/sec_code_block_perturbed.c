if (pixel_info == (MemoryInfo *) NULL)
    png_error(ping,"Allocation of memory for pixels failed");
  ping_pixels=(unsigned char *) GetVirtualMemoryBlob(pixel_info);
  (void) memset(ping_pixels,0,sizeof(rowbytes*sizeof(*ping_pixels)));
  /*
    Initialize image scanlines.
  */