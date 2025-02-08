if (prop->data != (uint8_t *) NULL)
      prop->data=(uint8_t *) RelinquishMagickMemory(prop->data);
    prop->data = (uint8_t *) AcquireCriticalMemory(prop->size);
    if (DBChop(&propDb, db, prop->size) != MagickTrue) {
      ThrowAndReturn("incorrect read size");
    }
    memcpy(prop->data, propDb.data, prop->size);

    switch (prop->type) {
      case ATOM('h', 'v', 'c', 'C'):
        ParseHvcCAtom(prop, exception);
        break;
      default:
        break;
    }