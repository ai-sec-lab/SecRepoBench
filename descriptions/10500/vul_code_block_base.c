if (pixel_info == (MemoryInfo *) NULL)
    png_error(ping,"Allocation of memory for pixels failed");
  ping_pixels=(unsigned char *) GetVirtualMemoryBlob(pixel_info);
  /*
    Initialize image scanlines.
  */