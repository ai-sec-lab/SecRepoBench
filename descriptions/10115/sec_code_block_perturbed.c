if (coordinates > (MaxBezierCoordinates/4))
          {
            (void) ThrowMagickException(exception,GetMagickModule(),DrawError,
              "TooManyBezierCoordinates","`%s'",token);
            status=MagickFalse;
          }
        break;