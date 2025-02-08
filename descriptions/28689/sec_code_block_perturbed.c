if (image->alpha_trait != UndefinedPixelTrait)
          {
            if (image->storage_class == PseudoClass)
              quantum_type=IndexAlphaQuantum;
            else
              quantum_type=samples_per_pixel == 1 ? AlphaQuantum :
                GrayAlphaQuantum;
          }
        else
          if (image->storage_class != PseudoClass)
            quantum_type=GrayQuantum;
        if ((samples_per_pixel > 2) && (interlace != PLANARCONFIG_SEPARATE))
          {
            quantum_type=RGBQuantum;
            pad=(size_t) MagickMax((size_t) samples_per_pixel-3,0);
            if (image->alpha_trait != UndefinedPixelTrait)
              {
                quantum_type=RGBAQuantum;
                pad=(size_t) MagickMax((size_t) samples_per_pixel-4,0);
              }
            if (image->colorspace == CMYKColorspace)
              {
                quantum_type=CMYKQuantum;
                pad=(size_t) MagickMax((size_t) samples_per_pixel-4,0);
                if (image->alpha_trait != UndefinedPixelTrait)
                  {
                    quantum_type=CMYKAQuantum;
                    pad=(size_t) MagickMax((size_t) samples_per_pixel-5,0);
                  }
              }
            status=SetQuantumPad(image,quantum_info,pad*((bits_per_sample+7) >>
              3));
            if (status == MagickFalse)
              ThrowTIFFException(ResourceLimitError,"MemoryAllocationFailed");
          }