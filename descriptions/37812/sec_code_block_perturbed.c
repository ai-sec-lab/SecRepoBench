switch( image->Coding ) {
	case VIPS_CODING_ERROR:
		isvalid = FALSE;
		break;

	case VIPS_CODING_LABQ:
		if( image->Type != VIPS_INTERPRETATION_LABQ )
			isvalid = FALSE;
		break;

	case VIPS_CODING_RAD:
		if( image->Type != VIPS_INTERPRETATION_sRGB )
			isvalid = FALSE;
		break;

	default:
		break;
	}

	switch( image->Type ) {
	case VIPS_INTERPRETATION_ERROR:
		isvalid = FALSE;
		break;

	case VIPS_INTERPRETATION_MULTIBAND: 
		/* This is a pretty useless generic tag. Always reset it.
		 */
		isvalid = FALSE;
		break;

	case VIPS_INTERPRETATION_B_W: 
		/* Don't test bands, we allow bands after the first to be
		 * unused extras, like alpha.
		 */
		break;

	case VIPS_INTERPRETATION_HISTOGRAM: 
		if( image->Xsize > 1 && image->Ysize > 1 )
			isvalid = FALSE;
		break;

	case VIPS_INTERPRETATION_FOURIER: 
		if( !vips_band_format_iscomplex( image->BandFmt ) )
			isvalid = FALSE;
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
			isvalid = FALSE;
		break;

	case VIPS_INTERPRETATION_CMYK: 
		if( image->Bands < 4 )
			isvalid = FALSE;
		break;

	case  VIPS_INTERPRETATION_LABQ:
		if( image->Coding != VIPS_CODING_LABQ )
			isvalid = FALSE;
		break;

	case  VIPS_INTERPRETATION_LABS:
		if( image->BandFmt != VIPS_FORMAT_SHORT )
			isvalid = FALSE;
		break;

	case  VIPS_INTERPRETATION_RGB16:
		if( image->BandFmt == VIPS_FORMAT_CHAR ||
			image->BandFmt == VIPS_FORMAT_UCHAR ||
			image->Bands < 3 )
			isvalid = FALSE;
		break;

	case  VIPS_INTERPRETATION_GREY16:
		if( image->BandFmt == VIPS_FORMAT_CHAR ||
			image->BandFmt == VIPS_FORMAT_UCHAR )
			isvalid = FALSE;
		break;

	case  VIPS_INTERPRETATION_MATRIX:
		if( image->Bands != 1 )
			isvalid = FALSE;
		break;

	default:
		g_assert_not_reached();
	}

	if( isvalid )
		return( vips_image_get_interpretation( image ) );
	else
		return( vips_image_default_interpretation( image ) );