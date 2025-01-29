Image
              *p,
              *q,
              *u,
              *v;

            ssize_t
              swap_index;

            index=(-1);
            swap_index=(-2);
            if (*option != '+')
              {
                GeometryInfo
                  geometry_info;

                MagickStatusType
                  flags;

                swap_index=(-1);
                flags=ParseGeometry(argv[i+1],&geometry_info);
                index=(ssize_t) geometry_info.rho;
                if ((flags & SigmaValue) != 0)
                  swap_index=(ssize_t) geometry_info.sigma;
              }
            p=GetImageFromList(*images,index);
            q=GetImageFromList(*images,swap_index);
            if ((p == (Image *) NULL) || (q == (Image *) NULL))
              {
                (void) ThrowMagickException(exception,GetMagickModule(),
                  OptionError,"NoSuchImage","`%s'",(*images)->filename);
                status=MagickFalse;
                break;
              }
            if (p == q)
              break;
            u=CloneImage(p,0,0,MagickTrue,exception);
            if (u == (Image *) NULL)
              break;
            v=CloneImage(q,0,0,MagickTrue,exception);
            if (v == (Image *) NULL)
              {
                u=DestroyImage(u);
                break;
              }
            ReplaceImageInList(&p,v);
            ReplaceImageInList(&q,u);