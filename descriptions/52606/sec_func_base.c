static int
vips_rot180_gen( VipsRegion *or, void *seq, void *a, void *b,
	gboolean *stop )
{
	VipsRegion *ir = (VipsRegion *) seq;
	VipsImage *in = (VipsImage *) a;

	/* Output area.
	 */
	VipsRect *r = &or->valid;
	int le = r->left;
	int ri = VIPS_RECT_RIGHT(r);
	int to = r->top;
	int bo = VIPS_RECT_BOTTOM(r);

	int x, y, i;

	/* Pixel geometry.
	 */
	int ps;

	/* Find the area of the input image we need.
	 */
	VipsRect need;

	need.left = in->Xsize - ri;
	need.top = in->Ysize - bo;
	need.width = r->width;
	need.height = r->height;
	if( vips_region_prepare( ir, &need ) )
		return( -1 );

	/* Find PEL size and line skip for ir.
	 */
	ps = VIPS_IMAGE_SIZEOF_PEL( in );

	/* Rotate the bit we now have.
	 */
	for( y = to; y < bo; y++ ) {
		/* Start of this output line.
		 */
		VipsPel *q = VIPS_REGION_ADDR( or, le, y );

		/* Corresponding position in ir.
		 */
		VipsPel *p = VIPS_REGION_ADDR( ir, 
			need.left + need.width - 1, 
			need.top + need.height - (y - to) - 1 );

		/* Blap across!
		 */
		for( x = le; x < ri; x++ ) {
			for( i = 0; i < ps; i++ )
				q[i] = p[i];

			q += ps;
			p -= ps;
		}
	}

	return( 0 );
}