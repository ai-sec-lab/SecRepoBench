/*
          Start a new stitch block.
        */
        j++;
        blocks[j].offset=(ssize_t) i;
        if (j >= 256)
          {
            stitches=(PointInfo *) RelinquishMagickMemory(stitches);
            ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
          }