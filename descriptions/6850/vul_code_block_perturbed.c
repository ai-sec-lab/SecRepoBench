GetPixelInfo(image,&pixel);
          fx_info=AcquireFxInfo(image,pattern+4,exception);
          value=0.0;
          status=FxEvaluateChannelExpression(fx_info,RedPixelChannel,0,0,
            &value,exception);