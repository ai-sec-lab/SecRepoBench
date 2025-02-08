FT_StreamRec
    stream;

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
  fterror=FT_Init_FreeType(&library);
  if (fterror != 0)
    ThrowFreetypeErrorException("UnableToInitializeFreetypeLibrary",fterror,
      image->filename);
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
  (void) memset(&stream,0,sizeof(stream));
  (void) stat(args.pathname,&attributes);
  stream.size=attributes.st_size;
  stream.descriptor.pointer=fopen_utf8(args.pathname,"rb");
  stream.read=(&FTReadStream);
  stream.close=(&FTCloseStream);
  args.flags=FT_OPEN_STREAM;
  args.stream=(&stream);