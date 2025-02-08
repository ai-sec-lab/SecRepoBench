RectangleInfo
        geometry;

      (void) ResetMagickMemory(&geometry,0,sizeof(geometry));
      flags=ParseAbsoluteGeometry(image_info->extract,&geometry);
      if (((flags & XValue) != 0) || ((flags & YValue) != 0))
        {
          image->extract_info=geometry;
          Swap(image->columns,image->extract_info.width);
          Swap(image->rows,image->extract_info.height);
        }