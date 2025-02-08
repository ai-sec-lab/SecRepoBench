im->BandFmt = VIPS_CLIP( 0, im->BandFmt, VIPS_FORMAT_LAST - 1 );
	im->Type = VIPS_CLIP( 0, im->Type, VIPS_INTERPRETATION_LAST - 1 );
	im->Coding = VIPS_CLIP( 0, im->Coding, VIPS_CODING_LAST - 1 );