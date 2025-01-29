ssize_t
                        count;

                      count=ReadBlob(image,(size_t) ldblk,BImgBuff);
                      if (count != ldblk)
                        break;