if (DBChop(&propDb, db, prop->size) != MagickTrue) {
      ThrowAndReturn("incorrect read size");
    }
    memcpy(prop->data, propDb.data, prop->size+4);