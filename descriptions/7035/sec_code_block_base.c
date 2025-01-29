if (status == MagickFalse)
              return(DestroyImageList(image));
            status=ResetImagePixels(image,exception);
            if (status == MagickFalse)
              return(DestroyImageList(image));
            break;