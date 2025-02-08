im->BandFmt = VIPS_CLIP( 0, im->BandFmt, VIPS_FORMAT_LAST - 1 );

	/* Coding and Type have missing values, so we look up in the enum.
	 */
	im->Type = g_enum_get_value( 
			g_type_class_ref( VIPS_TYPE_INTERPRETATION ), 
			im->Type ) ?
		im->Type : VIPS_INTERPRETATION_ERROR;
	im->Coding = g_enum_get_value( 
			g_type_class_ref( VIPS_TYPE_CODING ), 
			im->Coding ) ?
		im->Coding : VIPS_CODING_ERROR;