
        double
          pixel,
          scale;

        scale=QuantumRange/(double) ((1UL << jp2_image->comps[i].prec)-1);
        pixel=scale*(jp2_image->comps[i].data[y/jp2_image->comps[i].dy*
          image->columns/jp2_image->comps[i].dx+x/jp2_image->comps[i].dx]+
          (jp2_image->comps[i].sgnd ? 1UL << (jp2_image->comps[i].prec-1) : 0));
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
      