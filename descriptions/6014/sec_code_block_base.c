for (xml=p; *xml != '\0'; xml++)
      {
        char
          accept[] = " ";

        i=(ssize_t) strspn(xml,accept);
        if (i != 0)
          (void) CopyMagickMemory(xml,xml+i,strlen(xml+i)+1);
        while ((*xml != '\0') && (*xml != ' '))
          xml++;
        if (*xml == '\0')
          break;
      }
      xml--;
      if ((xml >= p) && (*xml == ' '))
        *xml='\0';