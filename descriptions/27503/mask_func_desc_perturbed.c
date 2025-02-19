int32_t
alac_decode (ALAC_DECODER *p, struct BitBuffer * bits, int32_t * sampleBuffer, uint32_t numSamples, uint32_t * outNumSamples)
{
	BitBuffer		shiftBits ;
	uint32_t		bits1, bits2 ;
	uint8_t			tag ;
	uint8_t			elementInstanceTag ;
	AGParamRec		agParams ;
	uint32_t		channelIndex ;
	int16_t			coefsU [32] ;		// max possible size is 32 although NUMCOEPAIRS is the current limit
	int16_t			coefsV [32] ;
	uint8_t			numU, numV ;
	uint8_t			mixBits ;
	int8_t			mixResiduals ;
	uint16_t		unusedHeader ;
	uint8_t			escapeFlag ;
	uint32_t		chanBits ;
	uint8_t			bytesShifted ;
	uint32_t		shift ;
	uint8_t			modeU, modeV ;
	uint32_t		denShiftU, denShiftV ;
	uint16_t		pbFactorU, pbFactorV ;
	uint16_t		pb ;
	int32_t *		out32 ;
	uint8_t			headerByte ;
	uint8_t			partialFrame ;
	uint32_t		extraBits ;
	int32_t			val ;
	uint32_t		i, j ;
	int32_t			status ;
	uint32_t		numChannels = p->mNumChannels ;

	RequireAction ((bits != NULL) && (sampleBuffer != NULL) && (outNumSamples != NULL), return kALAC_ParamError ;) ;
	RequireAction (p->mNumChannels > 0, return kALAC_ZeroChannelCount ;) ;

	p->mActiveElements = 0 ;
	channelIndex	= 0 ;

	status = ALAC_noErr ;
	*outNumSamples = numSamples ;

	while (status == ALAC_noErr)
	{
		// bail if we ran off the end of the buffer
		RequireAction (bits->cur < bits->end, status = kALAC_ParamError ; goto Exit ;) ;

		// copy global decode params for this element
		pb = p->mConfig.pb ;

		// read element tag
		tag = BitBufferReadSmall (bits, 3) ;
		// This section of code serves as a safeguard to break out of the decoding loop
		// once all channels have been processed, preventing further processing of
		// any remaining data bits that do not correspond to audio data. The condition
		// is wrapped in a preprocessor directive that allows enabling or disabling
		// this check in non-debug or debug builds respectively. This ensures that
		// in a production build, the code will stop decoding once the specified
		// number of channels has been reached, thereby avoiding potential errors
		// from unexpected data. In debug builds, the check can be disabled to allow
		// for more rigorous testing and detection of issues with the bitstream.
		// <MASK>
	}

NoMoreChannels:

	// if we get here and haven't decoded all of the requested channels, fill the remaining channels with zeros
	for ( ; channelIndex < numChannels ; channelIndex++)
	{
		int32_t *	fill32 = sampleBuffer + channelIndex ;
		Zero32 (fill32, numSamples, numChannels) ;
	}

Exit:
	return status ;
}