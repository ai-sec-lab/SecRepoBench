if (length < 7)
      return(MagickFalse);
    id=ReadProfileMSBShort(&p,&length);
    count=(ssize_t) ReadProfileByte(&p,&length);
    if ((count >= (ssize_t) length) || (count < 0))
      return(MagickFalse);
    p+=count;
    length-=count;