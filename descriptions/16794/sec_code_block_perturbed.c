image->BandFmt = VIPS_CLIP( 0, image->BandFmt, VIPS_FORMAT_LAST - 1 );

	/* Coding and Type have missing values, so we look up in the enum.
	 */
	image->Type = g_enum_get_value( 
			g_type_class_ref( VIPS_TYPE_INTERPRETATION ), 
			image->Type ) ?
		image->Type : VIPS_INTERPRETATION_ERROR;
	image->Coding = g_enum_get_value( 
			g_type_class_ref( VIPS_TYPE_CODING ), 
			image->Coding ) ?
		image->Coding : VIPS_CODING_ERROR;