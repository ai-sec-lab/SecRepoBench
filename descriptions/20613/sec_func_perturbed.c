static int
vips_foreign_load_gif_render( VipsForeignLoadGif *gifloader )
{
	GifFileType *file = gifloader->file;

	if( DGifGetImageDesc( file ) == GIF_ERROR ) {
		vips_foreign_load_gif_error( gifloader );
		return( -1 );
	}

	/* giflib does not check that the Left / Top / Width / Height for this
	 * Image is inside the canvas.
	 *
	 * We could clip against the canvas, but for now, just ignore out of
	 * bounds frames. Watch for int overflow too.
	 */
	if( file->Image.Left < 0 ||
		file->Image.Left > VIPS_MAX_COORD ||
		file->Image.Width <= 0 ||
		file->Image.Width > VIPS_MAX_COORD ||
		file->Image.Left + file->Image.Width > file->SWidth ||
		file->Image.Top < 0 ||
		file->Image.Top > VIPS_MAX_COORD ||
		file->Image.Height <= 0 ||
		file->Image.Height > VIPS_MAX_COORD ||
		file->Image.Top + file->Image.Height > file->SHeight ) {
		VIPS_DEBUG_MSG( "vips_foreign_load_gif_render: "
			"out of bounds frame of %d x %d pixels at %d x %d\n",
			file->Image.Width, file->Image.Height,
			file->Image.Left, file->Image.Top );

		/* Don't flag an error -- many GIFs have this problem.
		 */
		return( 0 );
	}

	/* Update the colour map for this frame.
	 */
	vips_foreign_load_gif_build_cmap( gifloader );

	/* PREVIOUS means we init the frame with the last un-disposed frame. 
	 * So the last un-disposed frame is used as a backdrop for the new 
	 * frame.
	 */
	if( gifloader->dispose == DISPOSE_PREVIOUS ) 
		memcpy( VIPS_IMAGE_ADDR( gifloader->scratch, 0, 0 ),
			VIPS_IMAGE_ADDR( gifloader->previous, 0, 0 ),
			VIPS_IMAGE_SIZEOF_IMAGE( gifloader->scratch ) );

	if( file->Image.Interlace ) {
		int i;

		VIPS_DEBUG_MSG( "vips_foreign_load_gif_render: "
			"interlaced frame of %d x %d pixels at %d x %d\n",
			file->Image.Width, file->Image.Height,
			file->Image.Left, file->Image.Top );

		for( i = 0; i < 4; i++ ) {
			int y;

			for( y = InterlacedOffset[i]; y < file->Image.Height; 
				y += InterlacedJumps[i] ) {
				VipsPel *dst = VIPS_IMAGE_ADDR( gifloader->scratch, 
					file->Image.Left, file->Image.Top + y );

				if( DGifGetLine( gifloader->file, 
					gifloader->line, file->Image.Width ) == 
						GIF_ERROR ) {
					vips_foreign_load_gif_error( gifloader );
					return( -1 );
				}

				vips_foreign_load_gif_render_line( gifloader, 
					file->Image.Width, dst, gifloader->line );
			}
		}
	}
	else {
		int y;

		VIPS_DEBUG_MSG( "vips_foreign_load_gif_render: "
			"non-interlaced frame of %d x %d pixels at %d x %d\n",
			file->Image.Width, file->Image.Height,
			file->Image.Left, file->Image.Top );

		for( y = 0; y < file->Image.Height; y++ ) {
			VipsPel *dst = VIPS_IMAGE_ADDR( gifloader->scratch, 
				file->Image.Left, file->Image.Top + y );

			if( DGifGetLine( gifloader->file, 
				gifloader->line, file->Image.Width ) == GIF_ERROR ) {
				vips_foreign_load_gif_error( gifloader );
				return( -1 );
			}

			vips_foreign_load_gif_render_line( gifloader, 
				file->Image.Width, dst, gifloader->line );
		}
	}

	/* Copy the result to frame, which then is picked up from outside
	 */
	memcpy( VIPS_IMAGE_ADDR( gifloader->frame, 0, 0 ),
		VIPS_IMAGE_ADDR( gifloader->scratch, 0, 0 ),
		VIPS_IMAGE_SIZEOF_IMAGE( gifloader->frame ) );

	if( gifloader->dispose == DISPOSE_BACKGROUND ) {
		/* BACKGROUND means we reset the frame to transparent before we
		 * render the next set of pixels.
		 */
		guint32 *q = (guint32 *) VIPS_IMAGE_ADDR( gifloader->scratch, 
			file->Image.Left, file->Image.Top );

		/* What we write for transparent pixels. We want RGB to be
		 * 255, and A to be 0.
		 */
		guint32 ink = GUINT32_TO_BE( 0xffffff00 );

		int x, y;

		/* Generate the first line a pixel at a time, memcpy() for
		 * subsequent lines.
		 */
		if( file->Image.Height > 0 ) 
			for( x = 0; x < file->Image.Width; x++ )
				q[x] = ink;

		for( y = 1; y < file->Image.Height; y++ )
			memcpy( q + gifloader->scratch->Xsize * y, 
				q, 
				file->Image.Width * sizeof( guint32 ) );
	}
	else if( gifloader->dispose == DISPOSAL_UNSPECIFIED || 
		gifloader->dispose == DISPOSE_DO_NOT ) 
		/* Copy the frame to previous, so it can be restored if 
		 * DISPOSE_PREVIOUS is specified in a later frame.
		 */
		memcpy( VIPS_IMAGE_ADDR( gifloader->previous, 0, 0 ),
			VIPS_IMAGE_ADDR( gifloader->frame, 0, 0 ),
			VIPS_IMAGE_SIZEOF_IMAGE( gifloader->previous ) );

	/* Reset values, as Graphic Control Extension is optional
	 */
	gifloader->dispose = DISPOSAL_UNSPECIFIED;
	gifloader->transparent_index = NO_TRANSPARENT_INDEX;

	return( 0 );
}