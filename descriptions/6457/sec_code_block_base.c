pcd_image=rotate_image;
      DestroyBlob(rotate_image);
      pcd_image->blob=ReferenceBlob(image->blob);