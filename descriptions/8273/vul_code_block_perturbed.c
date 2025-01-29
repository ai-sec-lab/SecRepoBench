clip_path=GetNodeByURL(primitive,token);
            if (clip_path != (char *) NULL)
              {
                if (graphic_context[n]->clipping_mask != (Image *) NULL)
                  graphic_context[n]->clipping_mask=
                    DestroyImage(graphic_context[n]->clipping_mask);
                graphic_context[n]->clipping_mask=DrawClippingMask(image,
                  graphic_context[n],token,clip_path,exception);
                clip_path=DestroyString(clip_path);
                if (draw_info->compliance != SVGCompliance)
                  status=SetImageMask(image,WritePixelMask,
                    graphic_context[n]->clipping_mask,exception);
              }
            break;