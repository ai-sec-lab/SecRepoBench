double
            value;

          FxInfo
            *fx_info;

          MagickStatusType
            status;

          PixelInfo
            pixel;

          /*
            Pixel - color value calculator.
          */
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
          if (image->colorspace == CMYKColorspace)
            {
              status&=FxEvaluateChannelExpression(fx_info,BlackPixelChannel,0,0,
                &value,exception);
              pixel.black=(double) QuantumRange*value;
            }
          status&=FxEvaluateChannelExpression(fx_info,AlphaPixelChannel,0,0,
            &value,exception);
          pixel.alpha=(double) QuantumRange*value;
          fx_info=DestroyFxInfo(fx_info);
          if (status != MagickFalse)
            {
              char
                hex[MagickPathExtent],
                name[MagickPathExtent];

              (void) QueryColorname(image,&pixel,SVGCompliance,name,exception);
              GetColorTuple(&pixel,MagickTrue,hex);
              AppendString2Text(hex+1);
            }
          continue;