clip_mask=CloneImage(image,image->columns,image->rows,MagickTrue,exception);
  if (clip_mask == (Image *) NULL)
    return(MagickFalse);
  (void) DeleteImageArtifact(image,filename);
  (void) SetImageMask(clip_mask,ReadPixelMask,(Image *) NULL,exception);
  (void) SetImageMask(clip_mask,WritePixelMask,(Image *) NULL,exception);
  (void) QueryColorCompliance("#0000",AllCompliance,
    &clip_mask->background_color,exception);