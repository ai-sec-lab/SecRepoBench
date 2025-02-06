if (strncmp(xml,"<!ENTITY",8) == 0)
      {
        /*
          Parse entity definitions.
        */
        xml+=strspn(xml+8,XMLWhitespace)+8;
        c=xml;
        n=xml+strspn(xml,XMLWhitespace "%");
        xml=n+strcspn(n,XMLWhitespace);
        *xml=';';
        v=xml+strspn(xml+1,XMLWhitespace)+1;
        q=(*v);
        v++;
        if ((q != '"') && (q != '\''))
          {
            /*
              Skip externals.
            */
            xml=strchr(xml,'>');
            continue;
          }
        entities=(*c == '%') ? predefined_entitites : root->entities;
        for (i=0; entities[i] != (char *) NULL; i++) ;
        entities=(char **) ResizeQuantumMemory(entities,(size_t) (i+3),
          sizeof(*entities));
        if (entities == (char **) NULL)
          ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
        if (*c == '%')
          predefined_entitites=entities;
        else
          root->entities=entities;
        xml++;
        *xml='\0';
        xml=strchr(v,q);
        if (xml != (char *) NULL)
          {
            *xml='\0';
            xml++;
          }
        entities[i+1]=ParseEntities(v,predefined_entitites,'%');
        entities[i+2]=(char *) NULL;
        if (ValidateEntities(n,entities[i+1],0,entities) != MagickFalse)
          entities[i]=n;
        else
          {
            if (entities[i+1] != v)
              entities[i+1]=DestroyString(entities[i+1]);
            (void) ThrowMagickException(exception,GetMagickModule(),
              OptionWarning,"ParseError","circular entity declaration &%s",n);
            predefined_entitites=(char **) RelinquishMagickMemory(
              predefined_entitites);
            return(MagickFalse);
          }
        }