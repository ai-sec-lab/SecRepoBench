if (image == (Image *) NULL)
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                OptionWarning,"NoImageForProperty","\"%%[%s]\"",pattern);
              continue; /* else no image to retrieve artifact */
            }
          if ((image->columns == 0) || (image->rows == 0))
            break;
          GetPixelInfo(image,&pixel);
          fx_info=AcquireFxInfo(image,pattern+4,exception);
          status=FxEvaluateChannelExpression(fx_info,RedPixelChannel,0,0,
            &value,exception);
          pixel.red=(double) QuantumRange*value;
          status&=FxEvaluateChannelExpression(fx_info,GreenPixelChannel,0,0,
            &value,exception);
          pixel.green=(double) QuantumRange*value;
          status&=FxEvaluateChannelExpression(fx_info,BluePixelChannel,0,0,
            &value,exception);
          pixel.blue=(double) QuantumRange*value;