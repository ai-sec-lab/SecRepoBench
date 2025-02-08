static int
vips_foreign_load_gif_render( VipsForeignLoadGif *gifloader )
{
	GifFileType *file = gifloader->file;

	if( DGifGetImageDesc( file ) == GIF_ERROR ) {
		vips_foreign_load_gif_error( gifloader );
		return( -1 );
	}

	// <MASK>

	/* Reset values, as Graphic Control Extension is optional
	 */
	gifloader->dispose = DISPOSAL_UNSPECIFIED;
	gifloader->transparent_index = NO_TRANSPARENT_INDEX;

	return( 0 );
}