for (xml=p; *xml != '\0'; xml++)
      {
        char
          accept[] = " ";

        index=(ssize_t) strspn(xml,accept);
        if (index != 0)
          (void) CopyMagickMemory(xml,xml+index,strlen(xml+index)+1);
        while ((*xml != '\0') && (*xml != ' '))
          xml++;
      }
      xml--;
      if ((xml >= p) && (*xml == ' '))
        *xml='\0';