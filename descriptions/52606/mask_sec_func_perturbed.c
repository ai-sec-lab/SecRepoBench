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
	int right = VIPS_RECT_RIGHT(r);
	int to = r->top;
	int bo = VIPS_RECT_BOTTOM(r);

	// <MASK>

	return( 0 );
}