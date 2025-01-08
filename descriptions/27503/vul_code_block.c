#if 0 // ! DEBUG
		// if we've decoded all of our channels, bail (but not in debug b/c we want to know if we're seeing bad bits)
		// - this also protects us if the config does not match the bitstream or crap data bits follow the audio bits
		if (channelIndex >= numChannels)
			break ;
#endif