PatchCorruptProfile(name,clone_profile);
  if ((LocaleCompare(name,"xmp") == 0) &&
      (ValidateXMPProfile(image,clone_profile,exception) == MagickFalse))
    {
      clone_profile=DestroyStringInfo(clone_profile);
      return(MagickTrue);
    }
  if (image->profiles == (SplayTreeInfo *) NULL)
    image->profiles=NewSplayTree(CompareSplayTreeString,RelinquishMagickMemory,
      DestroyProfile);
  (void) CopyMagickString(key,name,MagickPathExtent);
  LocaleLower(key);
  status=AddValueToSplayTree((SplayTreeInfo *) image->profiles,
    ConstantString(key),clone_profile);
  if (status != MagickFalse)
    {
      if (LocaleCompare(name,"8bim") == 0)
        GetProfilesFromResourceBlock(image,clone_profile,exception);
      else
        if (recursive == MagickFalse)
          WriteTo8BimProfile(image,name,clone_profile);
    }
  return(status);