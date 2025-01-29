if (strchr(draw_info->family,',') == (char *) NULL)
        type_info=GetTypeInfoByFamily(draw_info->family,draw_info->style,
          draw_info->stretch,draw_info->weight,exception);
      if (type_info == (const TypeInfo *) NULL)
        {
          char
            **family;

          int
            number_families;

          register ssize_t
            i;

          /*
            Parse font family list.
          */
          family=StringToArgv(draw_info->family,&number_families);
          for (i=1; i < (ssize_t) number_families; i++)
          {
            type_info=GetTypeInfoByFamily(family[i],draw_info->style,
              draw_info->stretch,draw_info->weight,exception);
            if ((type_info != (const TypeInfo *) NULL) &&
                (LocaleCompare(family[i],type_info->family) == 0))
              break;
          }
          for (i=0; i < (ssize_t) number_families; i++)
            family[i]=DestroyString(family[i]);
          family=(char **) RelinquishMagickMemory(family);
          if (type_info == (const TypeInfo *) NULL)
            (void) ThrowMagickException(exception,GetMagickModule(),TypeWarning,
              "UnableToReadFont","`%s'",draw_info->family);
        }