static MagickBooleanType ParseIpcoAtom(Image *image, DataBuffer *db,
    HEICImageContext *ctx, ExceptionInfo *errorinfo)
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
    if (prop->data != (uint8_t *) NULL)
      prop->data=(uint8_t *) RelinquishMagickMemory(prop->data);
    prop->data = (uint8_t *) AcquireCriticalMemory(prop->size);
    if (DBChop(&propDb, db, prop->size) != MagickTrue) {
      ThrowAndReturn("incorrect read size");
    }
    memcpy(prop->data, propDb.data, prop->size);

    switch (prop->type) {
      case ATOM('h', 'v', 'c', 'C'):
        ParseHvcCAtom(prop, errorinfo);
        break;
      default:
        break;
    }
  }

  return MagickTrue;
}