/*
              Get the keyword.
            */
            length=MagickPathExtent;
            p=keyword;
            do
            {
              if (c == (int) '=')
                break;
              if ((size_t) (p-keyword) < (MagickPathExtent-1))
                *p++=(char) c;
              c=ReadBlobByte(image);
            } while (c != EOF);