if (GetStringInfoLength(profile) < 17)
    return(MagickFalse);
  if ((property == (const char *) NULL) || (*property == '\0'))
    return(MagickFalse);
  xmp_profile=StringInfoToString(profile);
  if (xmp_profile == (char *) NULL)
    return(MagickFalse);
  if ((strstr(xmp_profile,"<rdf:RDF") == (char *) NULL) ||
      (strstr(xmp_profile,"<rdf:Description") == (char *) NULL) ||
      (strstr(xmp_profile,"<x:") == (char *) NULL))
    {
      xmp_profile=DestroyString(xmp_profile);
      return(MagickFalse);
    }
  for (p=xmp_profile; *p != '\0'; p++)
    if ((*p == '<') && (*(p+1) == 'x'))
      break;
  exception=AcquireExceptionInfo();
  xmp=NewXMLTree((char *) p,exception);
  xmp_profile=DestroyString(xmp_profile);
  exception=DestroyExceptionInfo(exception);
  if (xmp == (XMLTreeInfo *) NULL)
    return(MagickFalse);
  status=MagickFalse;
  rdf=GetXMLTreeChild(xmp,"rdf:RDF");
  if (rdf != (XMLTreeInfo *) NULL)
    {
      if (image->properties == (void *) NULL)
        ((Image *) image)->properties=NewSplayTree(CompareSplayTreeString,
          RelinquishMagickMemory,RelinquishMagickMemory);
      description=GetXMLTreeChild(rdf,"rdf:Description");
      while (description != (XMLTreeInfo *) NULL)
      {
        char
          *xmp_namespace;

        node=GetXMLTreeChild(description,(const char *) NULL);
        while (node != (XMLTreeInfo *) NULL)
        {
          child=GetXMLTreeChild(node,(const char *) NULL);
          content=GetXMLTreeContent(node);
          if ((child == (XMLTreeInfo *) NULL) &&
              (SkipXMPValue(content) == MagickFalse))
            {
              xmp_namespace=ConstantString(GetXMLTreeTag(node));
              (void) SubstituteString(&xmp_namespace,"exif:","xmp:");
              (void) AddValueToSplayTree((SplayTreeInfo *) image->properties,
                xmp_namespace,ConstantString(content));
            }
          while (child != (XMLTreeInfo *) NULL)
          {
            content=GetXMLTreeContent(child);
            if (SkipXMPValue(content) == MagickFalse)
              {
                xmp_namespace=ConstantString(GetXMLTreeTag(node));
                (void) SubstituteString(&xmp_namespace,"exif:","xmp:");
                (void) AddValueToSplayTree((SplayTreeInfo *) image->properties,
                  xmp_namespace,ConstantString(content));
              }
            child=GetXMLTreeSibling(child);
          }
          node=GetXMLTreeSibling(node);
        }
        description=GetNextXMLTreeTag(description);
      }
    }
  xmp=DestroyXMLTree(xmp);
  return(status);