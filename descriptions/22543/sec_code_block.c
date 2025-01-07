
        double
          pixel,
          scale;

        ssize_t data_index = y/jp2_image->comps[i].dy*
                             image->columns/jp2_image->comps[i].dx+
                             x/jp2_image->comps[i].dx;
        if(data_index < 0 ||
           data_index >= (jp2_image->comps[i].h * jp2_image->comps[i].w))
        {
          opj_destroy_codec(jp2_codec);
          opj_image_destroy(jp2_image);
          ThrowReaderException(CoderError,
                               "IrregularChannelGeometryNotSupported")
        }
        scale=QuantumRange/(double) ((1UL << jp2_image->comps[i].prec)-1);
        pixel=scale*(jp2_image->comps[i].data[data_index] +
              (jp2_image->comps[i].sgnd ?
               1UL << (jp2_image->comps[i].prec-1) : 0));
        switch (i)
        {
           case 0:
           {
             if (jp2_image->numcomps == 1)
               {
                 SetPixelGray(image,ClampToQuantum(pixel),q);
                 SetPixelAlpha(image,OpaqueAlpha,q);
                 break;
               }
             SetPixelRed(image,ClampToQuantum(pixel),q);
             SetPixelGreen(image,ClampToQuantum(pixel),q);
             SetPixelBlue(image,ClampToQuantum(pixel),q);
             SetPixelAlpha(image,OpaqueAlpha,q);
             break;
           }
           case 1:
           {
             if (jp2_image->numcomps == 2)
               {
                 SetPixelAlpha(image,ClampToQuantum(pixel),q);
                 break;
               }
             SetPixelGreen(image,ClampToQuantum(pixel),q);
             break;
           }
           case 2:
           {
             SetPixelBlue(image,ClampToQuantum(pixel),q);
             break;
           }
           case 3:
           {
             SetPixelAlpha(image,ClampToQuantum(pixel),q);
             break;
           }
        }
      