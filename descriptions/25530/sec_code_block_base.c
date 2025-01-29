/*
	**	Allocate enough space for 1 more than a multiple of 8 samples
	**	to avoid having to branch when pulling apart the nibbles.
	*/
	count = ((samplesperblock - 2) | 7) + 2 ;
	pimasize = sizeof (IMA_ADPCM_PRIVATE) + psf->sf.channels * (blockalign + samplesperblock + sizeof(short) * count) ;