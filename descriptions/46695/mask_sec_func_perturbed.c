static MagickBooleanType RenderFreetype(Image *img,const DrawInfo *draw_info,
  const char *encoding,const PointInfo *offset,TypeMetric *metrics,
  ExceptionInfo *exception)
{
#if !defined(FT_OPEN_PATHNAME)
#define FT_OPEN_PATHNAME  ft_open_pathname
#endif

#define ThrowFreetypeErrorException(tag,ft_status,value) \
{ \
  const char \
    *error_string=FreetypeErrorMessage(ft_status); \
  if (error_string != (const char *) NULL) \
    (void) ThrowMagickException(exception,GetMagickModule(),TypeError, \
      tag,"`%s (%s)'",value, error_string); \
  else \
    (void) ThrowMagickException(exception,GetMagickModule(),TypeError, \
      tag,"`%s'",value); \
}

  typedef struct _GlyphInfo
  {
    FT_UInt
      id;

    FT_Vector
      origin;

    FT_Glyph
      image;
  } GlyphInfo;

  char
    *p;

  const char
    *value;

  DrawInfo
    *annotate_info;

  FT_BBox
    bounds;

  FT_BitmapGlyph
    bitmap;

  FT_Encoding
    encoding_type;

  FT_Error
    ft_status;

  FT_Face
    face;

  FT_Int32
    flags;

  FT_Library
    library;

  FT_Long
    face_index;

  FT_Matrix
    affine;

  FT_Open_Args
    args;

  FT_StreamRec
    *stream;

  FT_UInt
    first_glyph_id,
    last_glyph_id,
    missing_glyph_id;

  FT_Vector
    origin;

  GlyphInfo
    glyph;

  GraphemeInfo
    *grapheme;

  MagickBooleanType
    status;

  PointInfo
    point,
    resolution;

  ssize_t
    i;

  size_t
    length;

  ssize_t
    code,
    last_character,
    y;

  static FT_Outline_Funcs
    OutlineMethods =
    {
      (FT_Outline_MoveTo_Func) TraceMoveTo,
      (FT_Outline_LineTo_Func) TraceLineTo,
      (FT_Outline_ConicTo_Func) TraceQuadraticBezier,
      (FT_Outline_CubicTo_Func) TraceCubicBezier,
      0, 0
    };

  struct stat
    attributes;

  unsigned char
    *utf8;

  /*
    Initialize Truetype library.
  */
  ft_status=FT_Init_FreeType(&library);
  if (ft_status != 0)
    ThrowFreetypeErrorException("UnableToInitializeFreetypeLibrary",ft_status,
      img->filename);
  /*
    Open font face.
  */
  face_index=(FT_Long) draw_info->face;
  (void) memset(&args,0,sizeof(args));
  if (draw_info->font == (char *) NULL)
    args.pathname=ConstantString("helvetica");
  else
    if (*draw_info->font != '@')
      args.pathname=ConstantString(draw_info->font);
    else
      {
        /*
          Extract face index, e.g. @msgothic[1].
        */
        ImageInfo *image_info = AcquireImageInfo();
        (void) strcpy(image_info->filename,draw_info->font+1);
        (void) SetImageInfo(image_info,0,exception);
        face_index=(FT_Long) image_info->scene;
        args.pathname=ConstantString(image_info->filename);
        image_info=DestroyImageInfo(image_info);
     }
  /*
    Configure streaming interface.
  */
  stream=(FT_StreamRec *) AcquireCriticalMemory(sizeof(*stream));
  (void) memset(stream,0,sizeof(*stream));
  (void) stat(args.pathname,&attributes);
  stream->size=attributes.st_size;
  stream->descriptor.pointer=fopen_utf8(args.pathname,"rb");
  stream->read=(&FTReadStream);
  stream->close=(&FTCloseStream);
  args.flags=FT_OPEN_STREAM;
  args.stream=stream;
  face=(FT_Face) NULL;
  ft_status=FT_Open_Face(library,&args,face_index,&face);
  if (ft_status != 0)
    {
      (void) FT_Done_FreeType(library);
      ThrowFreetypeErrorException("UnableToReadFont",ft_status,args.pathname);
      args.pathname=DestroyString(args.pathname);
      return(MagickFalse);
    }
  args.pathname=DestroyString(args.pathname);
  if ((draw_info->metrics != (char *) NULL) &&
      (IsPathAccessible(draw_info->metrics) != MagickFalse))
    (void) FT_Attach_File(face,draw_info->metrics);
  encoding_type=FT_ENCODING_UNICODE;
  ft_status=FT_Select_Charmap(face,encoding_type);
  if ((ft_status != 0) && (face->num_charmaps != 0))
    ft_status=FT_Set_Charmap(face,face->charmaps[0]);
  // <MASK>
}