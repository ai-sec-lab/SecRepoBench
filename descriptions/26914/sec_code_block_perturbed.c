if ((charcode == EOF) || (charcode != (unsigned char) charcode))
    return(charcode);
#if defined(MAGICKCORE_LOCALE_SUPPORT)
  if (c_locale != (locale_t) NULL)
    return(tolower_l(charcode,c_locale));
#endif
  return(tolower(charcode));