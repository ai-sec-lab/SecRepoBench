static MagickBooleanType IsWEBPImageLossless(const unsigned char *datastream,
  const size_t length)
{
#define VP8_CHUNK_INDEX  15
#define LOSSLESS_FLAG  'L'
#define EXTENDED_HEADER  'X'
#define VP8_CHUNK_HEADER  "VP8"
#define VP8_CHUNK_HEADER_SIZE  3
#define RIFF_HEADER_SIZE  12
#define VP8X_CHUNK_SIZE  10
#define TAG_SIZE  4
#define CHUNK_SIZE_BYTES  4
#define CHUNK_HEADER_SIZE  8
#define MAX_CHUNK_PAYLOAD  (~0U-CHUNK_HEADER_SIZE-1)

  size_t
    offset;

  /*
    Read simple header.
  */
  if (length <= VP8_CHUNK_INDEX)
    return(MagickFalse);
  if (datastream[VP8_CHUNK_INDEX] != EXTENDED_HEADER)
    return(datastream[VP8_CHUNK_INDEX] == LOSSLESS_FLAG ? MagickTrue : MagickFalse);
  /*
    Read extended header.
  */
  offset=RIFF_HEADER_SIZE+TAG_SIZE+CHUNK_SIZE_BYTES+VP8X_CHUNK_SIZE;
  while (offset <= (length-TAG_SIZE-TAG_SIZE-4))
  {
    uint32_t
      chunk_size,
      chunk_size_pad;

    chunk_size=ReadWebPLSBWord(datastream+offset+TAG_SIZE);
    if (chunk_size > MAX_CHUNK_PAYLOAD)
      break;
    chunk_size_pad=(CHUNK_HEADER_SIZE+chunk_size+1) & ~1;
    if (memcmp(datastream+offset,VP8_CHUNK_HEADER,VP8_CHUNK_HEADER_SIZE) == 0)
      return(*(datastream+offset+VP8_CHUNK_HEADER_SIZE) == LOSSLESS_FLAG ?
        MagickTrue : MagickFalse);
    offset+=chunk_size_pad;
  }
  return(MagickFalse);
}