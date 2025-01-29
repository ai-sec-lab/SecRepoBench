(void) memset(prop->data, 0, prop->size+4);
    if (DBChop(&propDb, db, prop->size) != MagickTrue) {
      ThrowAndReturn("incorrect read size");
    }
    memcpy(prop->data, propDb.data, prop->size);