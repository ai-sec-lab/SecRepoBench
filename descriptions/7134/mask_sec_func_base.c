static MagickBooleanType ParseIpcoAtom(Image *image, DataBuffer *db,
    HEICImageContext *ctx, ExceptionInfo *exception)
{
  unsigned int
    length, atom;

  HEICItemProp
    *prop;

  /*
     property indicies starts from 1
  */
  for (ctx->itemPropsCount = 1; ctx->itemPropsCount < MAX_ITEM_PROPS && DBGetSize(db) > 8; ctx->itemPropsCount++) {
    DataBuffer
      propDb;

    length = DBReadUInt(db);
    atom = DBReadUInt(db);

    if (ctx->itemPropsCount == MAX_ITEM_PROPS) {
      ThrowAndReturn("too many item properties");
    }

    prop = &(ctx->itemProps[ctx->itemPropsCount]);
    prop->type = atom;
    prop->size = length - 8;
    // <MASK>
  }

  return MagickTrue;
}