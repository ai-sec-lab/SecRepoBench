image->BandFmt = VIPS_CLIP( 0, image->BandFmt, VIPS_FORMAT_LAST - 1 );
	image->Type = VIPS_CLIP( 0, image->Type, VIPS_INTERPRETATION_LAST - 1 );
	image->Coding = VIPS_CLIP( 0, image->Coding, VIPS_CODING_LAST - 1 );