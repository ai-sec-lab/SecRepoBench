/*
          Start a new stitch block.
        */
        j++;
        blocks[j].offset=(ssize_t) i;
        if (j >= 255)
          {
            stitches=(PointInfo *) RelinquishMagickMemory(stitches);
            ThrowReaderException(ResourceLimitError,"CorruptImage");
          }