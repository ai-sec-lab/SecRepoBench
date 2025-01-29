char *dest_new;
                destlen *= 2;
                if (destlen > 0x2FFFE)
                  {
                    loglevel |= 1;
                    LOG_ERROR ("bit_TV_to_utf8: overlarge destlen %zu for %s",
                               destlen, source);
                    iconv_close (cd);
                    free (odest);
                    return NULL;
                  }
                dest_new = (char *)realloc (odest, destlen);
                if (dest_new)
                  odest = dest = dest_new;
                else
                  {
                    loglevel |= 1;
                    LOG_ERROR ("Out of memory");
                    iconv_close (cd);
                    free (odest);
                    return NULL;
                  }