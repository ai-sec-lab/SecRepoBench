if (clone_info->clip_mask != (char *) NULL)
    clone_info->clip_mask=DestroyString(clone_info->clip_mask);
  status=DrawImage(clip_mask,clone_info,errorinfo);
  (void) SetImageMask(image,WritePixelMask,clip_mask,errorinfo);
  clip_mask=DestroyImage(clip_mask);
  clone_info=DestroyDrawInfo(clone_info);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(DrawEvent,GetMagickModule(),"end clip-path");
  return(status != 0 ? MagickTrue : MagickFalse);