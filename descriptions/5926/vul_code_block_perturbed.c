if (tile_image == (Image *) NULL)
        ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
          image->filename);

      /* read in the tile */
      switch (inDocInfo->compression)
      {
        case COMPRESS_NONE:
          if (load_tile(image,tile_image,inDocInfo,inLayerInfo,(size_t) (offset2-offset),exception) == 0)
            status=MagickTrue;
          break;
        case COMPRESS_RLE:
          if (load_tile_rle (image,tile_image,inDocInfo,inLayerInfo,
              (int) (offset2-offset),exception) == 0)
            status=MagickTrue;
          break;
        case COMPRESS_ZLIB:
          tile_image=DestroyImage(tile_image);
          ThrowBinaryException(CoderError,"ZipCompressNotSupported",
            image->filename)
        case COMPRESS_FRACTAL:
          tile_image=DestroyImage(tile_image);
          ThrowBinaryException(CoderError,"FractalCompressNotSupported",
            image->filename)
      }