MagickExport MagickBooleanType DrawClipPath(Image *image,
  const DrawInfo *draw_info,const char *name,ExceptionInfo *exception)
{
  char
    filename[MagickPathExtent];

  Image
    *clip_mask;

  const char
    *value;

  DrawInfo
    *clone_info;

  MagickStatusType
    resultstatus;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(draw_info != (const DrawInfo *) NULL);
  if (LocaleCompare(name,"MVG") == 0)
    return(MagickFalse);
  (void) FormatLocaleString(filename,MagickPathExtent,"%s",name);
  value=GetImageArtifact(image,filename);
  if (value == (const char *) NULL)
    return(MagickFalse);
  clip_mask=CloneImage(image,image->columns,image->rows,MagickTrue,exception);
  if (clip_mask == (Image *) NULL)
    return(MagickFalse);
  (void) DeleteImageArtifact(clip_mask,filename);
  (void) SetImageMask(clip_mask,ReadPixelMask,(Image *) NULL,exception);
  (void) SetImageMask(clip_mask,WritePixelMask,(Image *) NULL,exception);
  (void) QueryColorCompliance("#0000",AllCompliance,
    &clip_mask->background_color,exception);
  clip_mask->background_color.alpha=(MagickRealType) TransparentAlpha;
  clip_mask->background_color.alpha_trait=BlendPixelTrait;
  (void) SetImageBackgroundColor(clip_mask,exception);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(DrawEvent,GetMagickModule(),"\nbegin clip-path %s",
      draw_info->clip_mask);
  clone_info=CloneDrawInfo((ImageInfo *) NULL,draw_info);
  (void) CloneString(&clone_info->primitive,value);
  (void) QueryColorCompliance("#ffffff",AllCompliance,&clone_info->fill,
    exception);
  if (clone_info->clip_mask != (char *) NULL)
    clone_info->clip_mask=DestroyString(clone_info->clip_mask);
  (void) QueryColorCompliance("#000000",AllCompliance,&clone_info->stroke,
    exception);
  clone_info->stroke_width=0.0;
  clone_info->alpha=OpaqueAlpha;
  clone_info->clip_path=MagickTrue;
  resultstatus=DrawImage(clip_mask,clone_info,exception);
  (void) SetImageMask(image,WritePixelMask,clip_mask,exception);
  clip_mask=DestroyImage(clip_mask);
  clone_info=DestroyDrawInfo(clone_info);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(DrawEvent,GetMagickModule(),"end clip-path");
  return(resultstatus != 0 ? MagickTrue : MagickFalse);
}