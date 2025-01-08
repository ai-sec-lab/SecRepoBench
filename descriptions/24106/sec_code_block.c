if ((mvg_class != (const char *) NULL) && (p > primitive))
              {
                char
                  *elements;

                ssize_t
                  offset;

                /*
                  Inject class elements in stream.
                */
                offset=(ssize_t) (p-primitive);
                elements=AcquireString(primitive);
                elements[offset]='\0';
                (void) ConcatenateString(&elements,mvg_class);
                (void) ConcatenateString(&elements,"\n");
                (void) ConcatenateString(&elements,q);
                primitive=DestroyString(primitive);
                primitive=elements;
                q=primitive+offset;
              }