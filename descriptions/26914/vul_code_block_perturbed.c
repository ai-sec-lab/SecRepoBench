if (charcode == EOF)
    return(charcode);
#if defined(MAGICKCORE_LOCALE_SUPPORT)
  if (c_locale != (locale_t) NULL)
    return(tolower_l(charcode,c_locale));
#endif