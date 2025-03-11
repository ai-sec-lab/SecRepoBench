static MagickBooleanType ParseInternalDoctype(XMLTreeRoot *root,char *xml,
  size_t length,ExceptionInfo *exception)
{
  char
    *c,
    **entities,
    *n,
    **predefined_entitites,
    q,
    *t,
    *v;

  register ssize_t
    i;

  ssize_t
    index;

  n=(char *) NULL;
  predefined_entitites=(char **) AcquireMagickMemory(sizeof(sentinel));
  if (predefined_entitites == (char **) NULL)
    ThrowFatalException(ResourceLimitError,"MemoryAllocationFailed");
  (void) CopyMagickMemory(predefined_entitites,sentinel,sizeof(sentinel));
  for (xml[length]='\0'; xml != (char *) NULL; )
  {
    while ((*xml != '\0') && (*xml != '<') && (*xml != '%'))
      xml++;
    if (*xml == '\0')
      break;
    if ((strlen(xml) > 9) && (strncmp(xml,"<!ENTITY",8) == 0))
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
      else
       if (strncmp(xml,"<!ATTLIST",9) == 0)
         {
            /*
              Parse default attributes.
            */
            t=xml+strspn(xml+9,XMLWhitespace)+9;
            if (*t == '\0')
              {
                (void) ThrowMagickException(exception,GetMagickModule(),
                  OptionWarning,"ParseError","unclosed <!ATTLIST");
                predefined_entitites=(char **) RelinquishMagickMemory(
                  predefined_entitites);
                return(MagickFalse);
              }
            xml=t+strcspn(t,XMLWhitespace ">");
            if (*xml == '>')
              continue;
            *xml='\0';
            i=0;
            while ((root->attributes[i] != (char **) NULL) &&
                   (n != (char *) NULL) &&
                   (strcmp(n,root->attributes[i][0]) != 0))
              i++;
            while ((*(n=xml+strspn(xml+1,XMLWhitespace)+1) != '\0') &&
                   (*n != '>'))
            {
              xml=n+strcspn(n,XMLWhitespace);
              if (*xml != '\0')
                *xml='\0';
              else
                {
                  (void) ThrowMagickException(exception,GetMagickModule(),
                    OptionWarning,"ParseError","malformed <!ATTLIST");
                  predefined_entitites=(char **) RelinquishMagickMemory(
                    predefined_entitites);
                  return(MagickFalse);
                }
              xml+=strspn(xml+1,XMLWhitespace)+1;
              c=(char *) (strncmp(xml,"CDATA",5) != 0 ? "*" : " ");
              if (strncmp(xml,"NOTATION",8) == 0)
                xml+=strspn(xml+8,XMLWhitespace)+8;
              xml=(*xml == '(') ? strchr(xml,')') : xml+
                strcspn(xml,XMLWhitespace);
              if (xml == (char *) NULL)
                {
                  (void) ThrowMagickException(exception,GetMagickModule(),
                    OptionWarning,"ParseError","malformed <!ATTLIST");
                  predefined_entitites=(char **) RelinquishMagickMemory(
                    predefined_entitites);
                  return(MagickFalse);
                }
              xml+=strspn(xml,XMLWhitespace ")");
              if (strncmp(xml,"#FIXED",6) == 0)
                xml+=strspn(xml+6,XMLWhitespace)+6;
              if (*xml == '#')
                {
                  xml+=strcspn(xml,XMLWhitespace ">")-1;
                  if (*c == ' ')
                    continue;
                  v=(char *) NULL;
                }
              else
                if (((*xml == '"') || (*xml == '\''))  &&
                    ((xml=strchr(v=xml+1,*xml)) != (char *) NULL))
                  *xml='\0';
                else
                  {
                    (void) ThrowMagickException(exception,GetMagickModule(),
                      OptionWarning,"ParseError","malformed <!ATTLIST");
                    predefined_entitites=(char **) RelinquishMagickMemory(
                      predefined_entitites);
                    return(MagickFalse);
                  }
              if (root->attributes[i] == (char **) NULL)
                {
                  /*
                    New attribute tag.
                  */
                  if (i == 0)
                    root->attributes=(char ***) AcquireQuantumMemory(2,
                      sizeof(*root->attributes));
                  else
                    root->attributes=(char ***) ResizeQuantumMemory(
                      root->attributes,(size_t) (i+2),
                      sizeof(*root->attributes));
                  if (root->attributes == (char ***) NULL)
                    ThrowFatalException(ResourceLimitFatalError,
                      "MemoryAllocationFailed");
                  root->attributes[i]=(char **) AcquireQuantumMemory(2,
                    sizeof(**root->attributes));
                  if (root->attributes[i] == (char **) NULL)
                    ThrowFatalException(ResourceLimitFatalError,
                      "MemoryAllocationFailed");
                  root->attributes[i][0]=ConstantString(t);
                  root->attributes[i][1]=(char *) NULL;
                  root->attributes[i+1]=(char **) NULL;
                }
              for (index=1; root->attributes[i][index] != (char *) NULL; index+=3) ;
              root->attributes[i]=(char **) ResizeQuantumMemory(
                root->attributes[i],(size_t) (index+4),sizeof(**root->attributes));
              if (root->attributes[i] == (char **) NULL)
                ThrowFatalException(ResourceLimitFatalError,
                  "MemoryAllocationFailed");
              root->attributes[i][index+3]=(char *) NULL;
              root->attributes[i][index+2]=ConstantString(c);
              root->attributes[i][index+1]=(char *) NULL;
              if (v != (char *) NULL)
                root->attributes[i][index+1]=ParseEntities(v,root->entities,*c);
              root->attributes[i][index]=ConstantString(n);
            }
        }
      else
        if (strncmp(xml, "<!--", 4) == 0)
          xml=strstr(xml+4,"-->");
        else
          if (strncmp(xml,"<?", 2) == 0)
            {
              c=xml+2;
              xml=strstr(c,"?>");
              if (xml != (char *) NULL)
                {
                  ParseProcessingInstructions(root,c,(size_t) (xml-c));
                  xml++;
                }
            }
           else
             if (*xml == '<')
               xml=strchr(xml,'>');
             else
               if ((*(xml++) == '%') && (root->standalone == MagickFalse))
                 break;
    }
  predefined_entitites=(char **) RelinquishMagickMemory(predefined_entitites);
  return(MagickTrue);
}