while (status != MagickFalse)
  {
    bytesread=zip_fread(merged_image_file,image_data_buffer+offset,
      sizeof(image_data_buffer)-offset);
    if (bytesread == -1)
      status=MagickFalse;
    else if (bytesread == 0)
      {
        /* Write up to offset of image_data_buffer to temp file */
        if (!fwrite(image_data_buffer,offset,1,file))
          status=MagickFalse;
        break;
      }
    else if (bytesread == (ssize_t) (sizeof(image_data_buffer)-offset))
      {
        /* Write the entirely of image_data_buffer to temp file */
        if (!fwrite(image_data_buffer,sizeof(image_data_buffer),1,file))
          status=MagickFalse;
        else
          offset=0;
      }
    else
      offset+=bytesread;
  }
  (void) fclose(file);
  (void) zip_fclose(merged_image_file);
  (void) zip_discard(zip_archive);
  if (status == MagickFalse)
    {
      ThrowFileException(exception,CoderError,"UnableToReadImageData",
          read_info->filename);
      (void) RelinquishUniqueFileResource(read_info->filename);
      read_info=DestroyImageInfo(read_info);
      image_metadata=DestroyImage(image_metadata);
      return((Image *) NULL);
    }
  /* Delegate to ReadImage to read mergedimage.png */
  out_image=ReadImage(read_info,exception);
  (void) RelinquishUniqueFileResource(read_info->filename);
  read_info=DestroyImageInfo(read_info);
  /* Update fields of image from fields of png_image */
  if (out_image != NULL)
    {
      (void) CopyMagickString(out_image->filename,image_metadata->filename,
        MagickPathExtent);
      (void) CopyMagickString(out_image->magick_filename,
        image_metadata->magick_filename,MagickPathExtent);
      out_image->timestamp=time(&stat_info.st_mtime);
      (void) CopyMagickString(out_image->magick,image_metadata->magick,
        MagickPathExtent);
      out_image->extent=stat_info.st_size;
    }