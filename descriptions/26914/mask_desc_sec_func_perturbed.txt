MagickExport int LocaleLowercase(const int charcode)
{
  // Convert a character to lowercase, considering locale settings if available.
  // If the character is EOF, return it unchanged.
  // If locale support is enabled and a C locale is available, use the locale-
  // specific function to convert the character to lowercase.
  // <MASK>
  return(tolower(charcode));
}