switch( image->Coding ) {
	case VIPS_CODING_LABQ:
		if( image->Type != VIPS_INTERPRETATION_LABQ )
			sane = FALSE;
		break;

	case VIPS_CODING_RAD:
		if( image->Type != VIPS_INTERPRETATION_sRGB )
			sane = FALSE;
		break;

	default:
		break;
	}

	switch( image->Type ) {
	case VIPS_INTERPRETATION_MULTIBAND: 
		/* This is a pretty useless generic tag. Always reset it.
		 */
		sane = FALSE;
		break;

	case VIPS_INTERPRETATION_B_W: 
		/* Don't test bands, we allow bands after the first to be
		 * unused extras, like alpha.
		 */
		break;

	case VIPS_INTERPRETATION_HISTOGRAM: 
		if( image->Xsize > 1 && image->Ysize > 1 )
			sane = FALSE;
		break;

	case VIPS_INTERPRETATION_FOURIER: 
		if( !vips_band_format_iscomplex( image->BandFmt ) )
			sane = FALSE;
		break;

	case VIPS_INTERPRETATION_XYZ: 
	case VIPS_INTERPRETATION_LAB: 
	case VIPS_INTERPRETATION_RGB: 
	case VIPS_INTERPRETATION_CMC: 
	case VIPS_INTERPRETATION_LCH: 
	case VIPS_INTERPRETATION_sRGB: 
	case VIPS_INTERPRETATION_HSV: 
	case VIPS_INTERPRETATION_scRGB: 
	case VIPS_INTERPRETATION_YXY: 
		if( image->Bands < 3 )
			sane = FALSE;
		break;

	case VIPS_INTERPRETATION_CMYK: 
		if( image->Bands < 4 )
			sane = FALSE;
		break;

	case  VIPS_INTERPRETATION_LABQ:
		if( image->Coding != VIPS_CODING_LABQ )
			sane = FALSE;
		break;

	case  VIPS_INTERPRETATION_LABS:
		if( image->BandFmt != VIPS_FORMAT_SHORT )
			sane = FALSE;
		break;

	case  VIPS_INTERPRETATION_RGB16:
		if( image->BandFmt == VIPS_FORMAT_CHAR ||
			image->BandFmt == VIPS_FORMAT_UCHAR ||
			image->Bands < 3 )
			sane = FALSE;
		break;

	case  VIPS_INTERPRETATION_GREY16:
		if( image->BandFmt == VIPS_FORMAT_CHAR ||
			image->BandFmt == VIPS_FORMAT_UCHAR )
			sane = FALSE;
		break;

	case  VIPS_INTERPRETATION_MATRIX:
		if( image->Bands != 1 )
			sane = FALSE;
		break;

	default:
		g_assert_not_reached();
	}

	if( sane )
		return( vips_image_get_interpretation( image ) );
	else
		return( vips_image_default_interpretation( image ) );