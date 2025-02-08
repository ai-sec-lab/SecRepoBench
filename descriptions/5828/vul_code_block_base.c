RectangleInfo
          geometry;

        flags=ParseAbsoluteGeometry(read_info->extract,&geometry);
        if ((next->columns != geometry.width) ||
            (next->rows != geometry.height))
          {
            if (((flags & XValue) != 0) || ((flags & YValue) != 0))
              {
                Image
                  *crop_image;

                crop_image=CropImage(next,&geometry,exception);
                if (crop_image != (Image *) NULL)
                  ReplaceImageInList(&next,crop_image);
              }
            else
              if (((flags & WidthValue) != 0) || ((flags & HeightValue) != 0))
                {
                  Image
                    *size_image;

                  flags=ParseRegionGeometry(next,read_info->extract,&geometry,
                    exception);
                  size_image=ResizeImage(next,geometry.width,geometry.height,
                    next->filter,exception);
                  if (size_image != (Image *) NULL)
                    ReplaceImageInList(&next,size_image);
                }
          }