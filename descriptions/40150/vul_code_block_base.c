double
          high,
          low;

        ssize_t
          n;

        /*
          Auto fit text into bounding box.
        */
        low=1.0;
        for (n=0; n < 32; n++, draw_info->pointsize*=2.0)
        {
          (void) FormatLocaleString(geometry,MagickPathExtent,"%+g%+g",
            metrics.bounds.x1,metrics.ascent);
          if (draw_info->gravity == UndefinedGravity)
            (void) CloneString(&draw_info->geometry,geometry);
          status=GetMultilineTypeMetrics(image,draw_info,&metrics,exception);
          if (status == MagickFalse)
            break;
          AdjustTypeMetricBounds(&metrics);
          width=(size_t) floor(metrics.width+draw_info->stroke_width+0.5);
          height=(size_t) floor(metrics.height+draw_info->stroke_width+0.5);
          if ((image->columns != 0) && (image->rows != 0))
            {
              if ((width >= image->columns) || (height >= image->rows))
                break;
              if ((width < image->columns) && (height < image->rows))
                low=draw_info->pointsize;
            }
          else
            if (((image->columns != 0) && (width >= image->columns)) ||
                ((image->rows != 0) && (height >= image->rows)))
              break;
        }
        if (status == MagickFalse)
          {
            label=DestroyString(label);
            draw_info=DestroyDrawInfo(draw_info);
            image=DestroyImageList(image);
            return((Image *) NULL);
          }