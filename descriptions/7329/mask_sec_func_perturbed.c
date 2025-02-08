static MagickBooleanType ParseIpcoAtom(Image *image, DataBuffer *db,
    HEICImageContext *heicCtx, ExceptionInfo *exception)
{
  unsigned int
    length, atom;

  HEICItemProp
    *prop;

  /*
     property indicies starts from 1
  */
  for (heicCtx->itemPropsCount = 1; heicCtx->itemPropsCount < MAX_ITEM_PROPS && DBGetSize(db) > 8; heicCtx->itemPropsCount++) {
    DataBuffer
      propDb;

    length = DBReadUInt(db);
    atom = DBReadUInt(db);

    if (heicCtx->itemPropsCount == MAX_ITEM_PROPS) {
      ThrowAndReturn("too many item properties");
    }

    prop = &(heicCtx->itemProps[heicCtx->itemPropsCount]);
    prop->type = atom;
    prop->size = length - 8;
    if (prop->size > DBGetSize(db))
      ThrowAndReturn("insufficient data");
    if (prop->data != (uint8_t *) NULL)
      prop->data=(uint8_t *) RelinquishMagickMemory(prop->data);
    prop->data = (uint8_t *) AcquireCriticalMemory(prop->size+4);
    // <MASK>

    switch (prop->type) {
      case ATOM('h', 'v', 'c', 'C'):
        ParseHvcCAtom(prop, exception);
        break;
      default:
        break;
    }
  }

  return MagickTrue;
}