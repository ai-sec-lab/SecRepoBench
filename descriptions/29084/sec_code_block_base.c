/*
          Convert stripped TIFF image.
        */
        extent=2*TIFFStripSize(tiff);
#if defined(TIFF_VERSION_BIG)
        extent+=image->columns*sizeof(uint64);
#else
        extent+=image->columns*sizeof(uint32);
#endif