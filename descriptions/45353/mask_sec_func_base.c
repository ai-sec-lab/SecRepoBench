static int
vips_foreign_load_pdf_header( VipsForeignLoad *load )
{
	VipsObjectClass *class = VIPS_OBJECT_GET_CLASS( load );
	VipsForeignLoadPdf *pdf = (VipsForeignLoadPdf *) load;

	int top;
	int i;

#ifdef DEBUG
	printf( "vips_foreign_load_pdf_header: %p\n", pdf );
#endif /*DEBUG*/

	g_mutex_lock( vips_pdfium_mutex );
	pdf->n_pages = FPDF_GetPageCount( pdf->doc );
	g_mutex_unlock( vips_pdfium_mutex );

	/* @n == -1 means until the end of the doc.
	 */
	if( pdf->n == -1 )
		pdf->n = pdf->n_pages - pdf->page_no;

	if( pdf->page_no + pdf->n > pdf->n_pages ||
		pdf->page_no < 0 ||
		pdf->n <= 0 ) {
		vips_error( class->nickname, "%s", _( "pages out of range" ) );
		return( -1 ); 
	}

	/* Lay out the pages in our output image.
	 */
	if( !(pdf->pages = VIPS_ARRAY( pdf, pdf->n, VipsRect )) )
		return( -1 ); 

	top = 0;
	pdf->image.left = 0;
	pdf->image.top = 0;
	pdf->image.width = 0;
	pdf->image.height = 0;
	for( i = 0; i < pdf->n; i++ ) {
		if( vips_foreign_load_pdf_get_page( pdf, pdf->page_no + i ) )
			return( -1 );
		pdf->pages[i].left = 0;
		pdf->pages[i].top = top;
		/* We do round to nearest, in the same way that vips_resize()
		 * does round to nearest. Without this, things like
		 * shrink-on-load will break.
		 */
		pdf->pages[i].width = VIPS_RINT( 
			FPDF_GetPageWidth( pdf->page ) * pdf->scale );
		pdf->pages[i].height = VIPS_RINT( 
			FPDF_GetPageHeight( pdf->page ) * pdf->scale );

		/* PDFium allows page width or height to be less than 1 (!!).
		 */
		// <MASK>
	}

	/* If all pages are the same height, we can tag this as a toilet roll
	 * image.
	 */
	for( i = 1; i < pdf->n; i++ ) 
		if( pdf->pages[i].height != pdf->pages[0].height )
			break;

	/* Only set page-height if we have more than one page, or this could
	 * accidentally turn into an animated image later.
	 */
	if( pdf->n > 1 )
		vips_image_set_int( load->out, 
			VIPS_META_PAGE_HEIGHT, pdf->pages[0].height );

	vips_foreign_load_pdf_set_image( pdf, load->out ); 

	/* Convert the background to the image format.
	 */
	if( !(pdf->ink = vips__vector_to_ink( class->nickname, 
		load->out, 
		VIPS_AREA( pdf->background )->data, NULL, 
		VIPS_AREA( pdf->background )->n )) )
		return( -1 );

	return( 0 );
}