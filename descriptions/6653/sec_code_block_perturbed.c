if (count != 6144)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  count=ReadBlob(image,768,(unsigned char *) zxattr);
  if (count != 768)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");