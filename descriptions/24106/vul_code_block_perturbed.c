if (LocaleCompare(token,graphic_context[graphiccontextindex]->id) == 0)
              break;
            mvg_class=(const char *) GetValueFromSplayTree(macros,token);
            if (mvg_class != (const char *) NULL)
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