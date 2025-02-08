static MagickBooleanType load_level(Image *image,XCFDocInfo *inDocInfo,
  XCFLayerInfo *inLayerInfo,ExceptionInfo *exception)
{
  int
    destLeft = 0,
    destTop = 0;

  Image*
    tile_image;

  MagickBooleanType
    status;

  MagickOffsetType
    currentpos,
    offset,
    offset2;

  register ssize_t
    i;

  size_t
    width,
    height,
    ntiles,
    ntile_rows,
    ntile_cols,
    tile_image_width,
    tile_image_height;

  /* start reading the data */
  width=ReadBlobMSBLong(image);
  height=ReadBlobMSBLong(image);

  /* read in the first tile offset.
   *  if it is '0', then this tile level is empty
   *  and we can simply return.
   */
  offset=(MagickOffsetType) ReadBlobMSBLong(image);
  if (offset == 0)
    return(MagickTrue);
  /* Initialise the reference for the in-memory tile-compression
   */
  ntile_rows=(height+TILE_HEIGHT-1)/TILE_HEIGHT;
  ntile_cols=(width+TILE_WIDTH-1)/TILE_WIDTH;
  ntiles=ntile_rows*ntile_cols;
  for (i = 0; i < (ssize_t) ntiles; i++)
  {
    status=MagickFalse;
    if (offset == 0)
      ThrowBinaryException(CorruptImageError,"NotEnoughTiles",image->filename);
    /* save the current position as it is where the
     *  next tile offset is stored.
     */
    currentpos=TellBlob(image);
    /* read in the offset of the next tile so we can calculate the amount
       of data needed for this tile*/
    offset2=(MagickOffsetType)ReadBlobMSBLong(image);
    /* if the offset is 0 then we need to read in the maximum possible
       allowing for negative compression */
    if (offset2 == 0)
      offset2=(MagickOffsetType) (offset + TILE_WIDTH * TILE_WIDTH * 4* 1.5);
    /* seek to the tile offset */
    if (SeekBlob(image, offset, SEEK_SET) != offset)
      ThrowBinaryException(CorruptImageError,"InsufficientImageDataInFile",
        image->filename);

      /*
        Allocate the image for the tile.  NOTE: the last tile in a row or
        column may not be a full tile!
      */
      tile_image_width=(size_t) (destLeft == (int) ntile_cols-1 ?
        (int) width % TILE_WIDTH : TILE_WIDTH);
      if (tile_image_width == 0)
        tile_image_width=TILE_WIDTH;
      tile_image_height = (size_t) (destTop == (int) ntile_rows-1 ?
        (int) height % TILE_HEIGHT : TILE_HEIGHT);
      if (tile_image_height == 0)
        tile_image_height=TILE_HEIGHT;
      tile_image=CloneImage(inLayerInfo->image,tile_image_width,
        tile_image_height,MagickTrue,exception);
      // <MASK>

      /* composite the tile onto the layer's image, and then destroy it */
      (void) CompositeImage(inLayerInfo->image,tile_image,CopyCompositeOp,
        MagickTrue,destLeft * TILE_WIDTH,destTop*TILE_HEIGHT,exception);
      tile_image=DestroyImage(tile_image);

      /* adjust tile position */
      destLeft++;
      if (destLeft >= (int) ntile_cols)
        {
          destLeft = 0;
          destTop++;
        }
      if (status != MagickFalse)
        return(MagickFalse);
      /* restore the saved position so we'll be ready to
       *  read the next offset.
       */
      offset=SeekBlob(image, currentpos, SEEK_SET);
      /* read in the offset of the next tile */
      offset=(MagickOffsetType) ReadBlobMSBLong(image);
    }
  if (offset != 0)
    ThrowBinaryException(CorruptImageError,"CorruptImage",image->filename)
  return(MagickTrue);
}