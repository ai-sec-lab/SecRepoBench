/* repeat previous line runcount* */
          c=ReadBlobByte(image);
          if (c == EOF)
            {
              BImgBuff=(unsigned char *) RelinquishMagickMemory(BImgBuff);
              return(-7);
            }
          RunCount=(unsigned char) c;
          if(x) {    /* attempt to duplicate row from x position: */
            /* I do not know what to do here */
            BImgBuff=(unsigned char *) RelinquishMagickMemory(BImgBuff);
            return(-3);
          }
          for(i=0;i < (int) RunCount;i++)
            {
              x=0;
              rowIndex++;    /* Here I need to duplicate previous row RUNCOUNT* */
              if(rowIndex<2) continue;
              if(rowIndex>(ssize_t) image->rows)
                {
                  BImgBuff=(unsigned char *) RelinquishMagickMemory(BImgBuff);
                  return(-4);
                }
              if (InsertRow(image,BImgBuff,rowIndex-1,bpp,exception) == MagickFalse)
                {
                  BImgBuff=(unsigned char *) RelinquishMagickMemory(BImgBuff);
                  return(-5);
                }
            }