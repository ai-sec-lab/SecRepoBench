clip_mask=CloneImage(image,image->columns,image->rows,MagickTrue,exception);
  if (clip_mask == (Image *) NULL)
    return(MagickFalse);
  (void) DeleteImageArtifact(clip_mask,filename);
  (void) SetImageMask(clip_mask,ReadPixelMask,(Image *) NULL,exception);
  (void) SetImageMask(clip_mask,WritePixelMask,(Image *) NULL,exception);
  (void) QueryColorCompliance("#0000",AllCompliance,
    &clip_mask->background_color,exception);