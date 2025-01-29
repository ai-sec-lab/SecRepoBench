if ((image->columns == 0) || (image->rows == 0))
            break;
          GetPixelInfo(image,&pixel);
          fx_info=AcquireFxInfo(image,pattern+4,exception);
          status=FxEvaluateChannelExpression(fx_info,RedPixelChannel,0,0,
            &value,exception);