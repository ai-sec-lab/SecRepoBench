if (image == (Image *) NULL)
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                OptionWarning,"NoImageForProperty","\"%%[%s]\"",pattern);
              continue; /* else no image to retrieve artifact */
            }
          GetPixelInfo(image,&pixel);
          fx_info=AcquireFxInfo(image,pattern+4,exception);
          value=0.0;
          status=FxEvaluateChannelExpression(fx_info,RedPixelChannel,0,0,
            &value,exception);
          pixel.red=(double) QuantumRange*value;
          status&=FxEvaluateChannelExpression(fx_info,GreenPixelChannel,0,0,
            &value,exception);
          pixel.green=(double) QuantumRange*value;
          status&=FxEvaluateChannelExpression(fx_info,BluePixelChannel,0,0,
            &value,exception);
          pixel.blue=(double) QuantumRange*value;