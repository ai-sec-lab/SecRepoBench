static int
vips_foreign_load_gif_render( VipsForeignLoadGif *gif )
{
	GifFileType *file = gif->file;

	if( DGifGetImageDesc( file ) == GIF_ERROR ) {
		vips_foreign_load_gif_error( gif );
		return( -1 );
	}

	// <MASK>

	/* Reset values, as Graphic Control Extension is optional
	 */
	gif->dispose = DISPOSAL_UNSPECIFIED;
	gif->transparent_index = NO_TRANSPARENT_INDEX;

	return( 0 );
}