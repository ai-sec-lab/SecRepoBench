static Image *ReadPSImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
#define BoundingBox  "BoundingBox:"
#define BeginDocument  "BeginDocument:"
#define BeginXMPPacket  "<?xpacket begin="
#define EndXMPPacket  "<?xpacket end="
#define ICCProfile "BeginICCProfile:"
#define CMYKCustomColor  "CMYKCustomColor:"
#define CMYKProcessColor  "CMYKProcessColor:"
#define DocumentMedia  "DocumentMedia:"
#define DocumentCustomColors  "DocumentCustomColors:"
#define DocumentProcessColors  "DocumentProcessColors:"
#define EndDocument  "EndDocument:"
#define HiResBoundingBox  "HiResBoundingBox:"
#define ImageData  "ImageData:"
#define PageBoundingBox  "PageBoundingBox:"
#define LanguageLevel  "LanguageLevel:"
#define PageMedia  "PageMedia:"
#define Pages  "Pages:"
#define PhotoshopProfile  "BeginPhotoshop:"
#define PostscriptLevel  "!PS-"
#define RenderPostscriptText  "  Rendering Postscript...  "
#define SpotColor  "+ "

  char
    command[MagickPathExtent],
    *density,
    filename[MagickPathExtent],
    geometry[MagickPathExtent],
    input_filename[MagickPathExtent],
    message[MagickPathExtent],
    *options,
    postscript_filename[MagickPathExtent];

  const char
    *option;

  const DelegateInfo
    *delegate_info;

  GeometryInfo
    geometry_info;

  Image
    *image,
    *next,
    *postscript_image;

  ImageInfo
    *read_info;

  int
    c,
    file;

  // <MASK>
  do
  {
    (void) CopyMagickString(postscript_image->filename,filename,
      MagickPathExtent);
    (void) CopyMagickString(postscript_image->magick,image->magick,
      MagickPathExtent);
    if (columns != 0)
      postscript_image->magick_columns=columns;
    if (rows != 0)
      postscript_image->magick_rows=rows;
    postscript_image->page=page;
    (void) CloneImageProfiles(postscript_image,image);
    (void) CloneImageProperties(postscript_image,image);
    next=SyncNextImageInList(postscript_image);
    if (next != (Image *) NULL)
      postscript_image=next;
  } while (next != (Image *) NULL);
  image=DestroyImageList(image);
  scene=0;
  for (next=GetFirstImageInList(postscript_image); next != (Image *) NULL; )
  {
    next->scene=scene++;
    next=GetNextImageInList(next);
  }
  return(GetFirstImageInList(postscript_image));
}