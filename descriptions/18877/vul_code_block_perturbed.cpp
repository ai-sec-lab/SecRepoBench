if (strips.front().n != 0 || strips.back().n != (mRaw->dim.y - 1))
    ThrowRDE("Strips validation issue.");
  // All good.