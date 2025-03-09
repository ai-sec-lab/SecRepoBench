switch (tag)
		{
			case ID_SCE:
			case ID_LFE:
			{
				// mono/LFE channel
				elementInstanceTag = BitBufferReadSmall (bits, 4) ;
				p->mActiveElements |= (1u << elementInstanceTag) ;

				// read the 12 unused header bits
				unusedHeader = (uint16_t) BitBufferRead (bits, 12) ;
				RequireAction (unusedHeader == 0, status = kALAC_ParamError ; goto Exit ;) ;

				// read the 1-bit "partial frame" flag, 2-bit "shift-off" flag & 1-bit "escape" flag
				headerByte = (uint8_t) BitBufferRead (bits, 4) ;

				partialFrame = headerByte >> 3 ;

				bytesShifted = (headerByte >> 1) & 0x3u ;
				RequireAction (bytesShifted != 3, status = kALAC_ParamError ; goto Exit ;) ;

				shift = bytesShifted * 8 ;

				escapeFlag = headerByte & 0x1 ;

				chanBits = p->mConfig.bitDepth - (bytesShifted * 8) ;

				// check for partial frame to override requested numSamples
				if (partialFrame != 0)
				{
					numSamples = BitBufferRead (bits, 16) << 16 ;
					numSamples |= BitBufferRead (bits, 16) ;

					RequireAction (numSamples < kALACDefaultFramesPerPacket, return kALAC_NumSamplesTooBig ;) ;
				}

				if (escapeFlag == 0)
				{
					// compressed frame, read rest of parameters
					mixBits	= (uint8_t) BitBufferRead (bits, 8) ;
					mixResiduals	= (int8_t) BitBufferRead (bits, 8) ;
					//Assert ((mixBits == 0) && (mixRes == 0)) ;		// no mixing for mono

					headerByte	= (uint8_t) BitBufferRead (bits, 8) ;
					modeU		= headerByte >> 4 ;
					denShiftU	= headerByte & 0xfu ;

					headerByte	= (uint8_t) BitBufferRead (bits, 8) ;
					pbFactorU	= headerByte >> 5 ;
					numU		= headerByte & 0x1fu ;

					for (i = 0 ; i < numU ; i++)
						coefsU [i] = (int16_t) BitBufferRead (bits, 16) ;

					// if shift active, skip the shift buffer but remember where it starts
					if (bytesShifted != 0)
					{
						shiftBits = *bits ;
						BitBufferAdvance (bits, (bytesShifted * 8) * numSamples) ;
					}

					// decompress
					set_ag_params (&agParams, p->mConfig.mb, (pb * pbFactorU) / 4, p->mConfig.kb, numSamples, numSamples, p->mConfig.maxRun) ;
					status = dyn_decomp (&agParams, bits, p->u.mPredictor, numSamples, chanBits, &bits1) ;
					RequireNoErr (status, goto Exit ;) ;

					if (modeU == 0)
					{
						unpc_block (p->u.mPredictor, p->mMixBufferU, numSamples, &coefsU [0], numU, chanBits, denShiftU) ;
					}
					else
					{
						// the special "numActive == 31" mode can be done in-place
						unpc_block (p->u.mPredictor, p->u.mPredictor, numSamples, NULL, 31, chanBits, 0) ;
						unpc_block (p->u.mPredictor, p->mMixBufferU, numSamples, &coefsU [0], numU, chanBits, denShiftU) ;
					}
				}
				else
				{
					//Assert (bytesShifted == 0) ;

					// uncompressed frame, copy data into the mix buffer to use common output code
					shift = 32 - chanBits ;
					if (chanBits <= 16)
					{
						for (i = 0 ; i < numSamples ; i++)
						{
							val = (int32_t) BitBufferRead (bits, (uint8_t) chanBits) ;
							val = (val << shift) >> shift ;
							p->mMixBufferU [i] = val ;
						}
					}
					else
					{
						// BitBufferRead () can't read more than 16 bits at a time so break up the reads
						extraBits = chanBits - 16 ;
						for (i = 0 ; i < numSamples ; i++)
						{
							val = (int32_t) BitBufferRead (bits, 16) ;
							val = arith_shift_left (val, 16) >> shift ;
							p->mMixBufferU [i] = val | BitBufferRead (bits, (uint8_t) extraBits) ;
						}
					}

					mixBits = mixResiduals = 0 ;
					bits1 = chanBits * numSamples ;
					bytesShifted = 0 ;
				}

				// now read the shifted values into the shift buffer
				if (bytesShifted != 0)
				{
					shift = bytesShifted * 8 ;
					//Assert (shift <= 16) ;

					for (i = 0 ; i < numSamples ; i++)
						p->u.mShiftBuffer [i] = (uint16_t) BitBufferRead (&shiftBits, (uint8_t) shift) ;
				}

				// convert 32-bit integers into output buffer
				switch (p->mConfig.bitDepth)
				{
					case 16:
						out32 = sampleBuffer + channelIndex ;
						for (i = 0, j = 0 ; i < numSamples ; i++, j += numChannels)
							out32 [j] = arith_shift_left (p->mMixBufferU [i], 16) ;
						break ;
					case 20:
						out32 = sampleBuffer + channelIndex ;
						copyPredictorTo20 (p->mMixBufferU, out32, numChannels, numSamples) ;
						break ;
					case 24:
						out32 = sampleBuffer + channelIndex ;
						if (bytesShifted != 0)
							copyPredictorTo24Shift (p->mMixBufferU, p->u.mShiftBuffer, out32, numChannels, numSamples, bytesShifted) ;
						else
							copyPredictorTo24 (p->mMixBufferU, out32, numChannels, numSamples) ;
						break ;
					case 32:
						out32 = sampleBuffer + channelIndex ;
						if (bytesShifted != 0)
							copyPredictorTo32Shift (p->mMixBufferU, p->u.mShiftBuffer, out32, numChannels, numSamples, bytesShifted) ;
						else
							copyPredictorTo32 (p->mMixBufferU, out32, numChannels, numSamples) ;
						break ;
				}

				channelIndex += 1 ;
				*outNumSamples = numSamples ;
				break ;
			}

			case ID_CPE:
			{
				// if decoding this pair would take us over the max channels limit, bail
				if ((channelIndex + 2) > numChannels)
					goto NoMoreChannels ;

				// stereo channel pair
				elementInstanceTag = BitBufferReadSmall (bits, 4) ;
				p->mActiveElements |= (1u << elementInstanceTag) ;

				// read the 12 unused header bits
				unusedHeader = (uint16_t) BitBufferRead (bits, 12) ;
				RequireAction (unusedHeader == 0, status = kALAC_ParamError ; goto Exit ;) ;

				// read the 1-bit "partial frame" flag, 2-bit "shift-off" flag & 1-bit "escape" flag
				headerByte = (uint8_t) BitBufferRead (bits, 4) ;

				partialFrame = headerByte >> 3 ;

				bytesShifted = (headerByte >> 1) & 0x3u ;
				RequireAction (bytesShifted != 3, status = kALAC_ParamError ; goto Exit ;) ;

				shift = bytesShifted * 8 ;

				escapeFlag = headerByte & 0x1 ;

				chanBits = p->mConfig.bitDepth - (bytesShifted * 8) + 1 ;

				// check for partial frame length to override requested numSamples
				if (partialFrame != 0)
				{
					numSamples = BitBufferRead (bits, 16) << 16 ;
					numSamples |= BitBufferRead (bits, 16) ;

					RequireAction (numSamples < kALACDefaultFramesPerPacket, return kALAC_NumSamplesTooBig ;) ;
				}

				if (escapeFlag == 0)
				{
					// compressed frame, read rest of parameters
					mixBits		= (uint8_t) BitBufferRead (bits, 8) ;
					mixResiduals		= (int8_t) BitBufferRead (bits, 8) ;

					headerByte	= (uint8_t) BitBufferRead (bits, 8) ;
					modeU		= headerByte >> 4 ;
					denShiftU	= headerByte & 0xfu ;

					headerByte	= (uint8_t) BitBufferRead (bits, 8) ;
					pbFactorU	= headerByte >> 5 ;
					numU		= headerByte & 0x1fu ;
					for (i = 0 ; i < numU ; i++)
						coefsU [i] = (int16_t) BitBufferRead (bits, 16) ;

					headerByte	= (uint8_t) BitBufferRead (bits, 8) ;
					modeV		= headerByte >> 4 ;
					denShiftV	= headerByte & 0xfu ;

					headerByte	= (uint8_t) BitBufferRead (bits, 8) ;
					pbFactorV	= headerByte >> 5 ;
					numV		= headerByte & 0x1fu ;
					for (i = 0 ; i < numV ; i++)
						coefsV [i] = (int16_t) BitBufferRead (bits, 16) ;

					// if shift active, skip the interleaved shifted values but remember where they start
					if (bytesShifted != 0)
					{
						shiftBits = *bits ;
						BitBufferAdvance (bits, (bytesShifted * 8) * 2 * numSamples) ;
					}

					// decompress and run predictor for "left" channel
					set_ag_params (&agParams, p->mConfig.mb, (pb * pbFactorU) / 4, p->mConfig.kb, numSamples, numSamples, p->mConfig.maxRun) ;
					status = dyn_decomp (&agParams, bits, p->u.mPredictor, numSamples, chanBits, &bits1) ;
					RequireNoErr (status, goto Exit ;) ;

					if (modeU == 0)
					{
						unpc_block (p->u.mPredictor, p->mMixBufferU, numSamples, &coefsU [0], numU, chanBits, denShiftU) ;
					}
					else
					{
						// the special "numActive == 31" mode can be done in-place
						unpc_block (p->u.mPredictor, p->u.mPredictor, numSamples, NULL, 31, chanBits, 0) ;
						unpc_block (p->u.mPredictor, p->mMixBufferU, numSamples, &coefsU [0], numU, chanBits, denShiftU) ;
					}

					// decompress and run predictor for "right" channel
					set_ag_params (&agParams, p->mConfig.mb, (pb * pbFactorV) / 4, p->mConfig.kb, numSamples, numSamples, p->mConfig.maxRun) ;
					status = dyn_decomp (&agParams, bits, p->u.mPredictor, numSamples, chanBits, &bits2) ;
					RequireNoErr (status, goto Exit ;) ;

					if (modeV == 0)
					{
						unpc_block (p->u.mPredictor, p->mMixBufferV, numSamples, &coefsV [0], numV, chanBits, denShiftV) ;
					}
					else
					{
						// the special "numActive == 31" mode can be done in-place
						unpc_block (p->u.mPredictor, p->u.mPredictor, numSamples, NULL, 31, chanBits, 0) ;
						unpc_block (p->u.mPredictor, p->mMixBufferV, numSamples, &coefsV [0], numV, chanBits, denShiftV) ;
					}
				}
				else
				{
					//Assert (bytesShifted == 0) ;

					// uncompressed frame, copy data into the mix buffers to use common output code
					chanBits = p->mConfig.bitDepth ;
					shift = 32 - chanBits ;
					if (chanBits <= 16)
					{
						for (i = 0 ; i < numSamples ; i++)
						{
							val = (int32_t) BitBufferRead (bits, (uint8_t) chanBits) ;
							val = (val << shift) >> shift ;
							p->mMixBufferU [i] = val ;

							val = (int32_t) BitBufferRead (bits, (uint8_t) chanBits) ;
							val = (val << shift) >> shift ;
							p->mMixBufferV [i] = val ;
						}
					}
					else
					{
						// BitBufferRead () can't read more than 16 bits at a time so break up the reads
						extraBits = chanBits - 16 ;
						for (i = 0 ; i < numSamples ; i++)
						{
							val = (int32_t) BitBufferRead (bits, 16) ;
							val = (((uint32_t) val) << 16) >> shift ;
							p->mMixBufferU [i] = val | BitBufferRead (bits, (uint8_t) extraBits) ;

							val = (int32_t) BitBufferRead (bits, 16) ;
							val = ((uint32_t) val) >> shift ;
							p->mMixBufferV [i] = val | BitBufferRead (bits, (uint8_t) extraBits) ;
						}
					}

					bits1 = chanBits * numSamples ;
					bits2 = chanBits * numSamples ;
					mixBits = mixResiduals = 0 ;
					bytesShifted = 0 ;
				}

				// now read the shifted values into the shift buffer
				if (bytesShifted != 0)
				{
					shift = bytesShifted * 8 ;
					//Assert (shift <= 16) ;

					for (i = 0 ; i < (numSamples * 2) ; i += 2)
					{
						p->u.mShiftBuffer [i + 0] = (uint16_t) BitBufferRead (&shiftBits, (uint8_t) shift) ;
						p->u.mShiftBuffer [i + 1] = (uint16_t) BitBufferRead (&shiftBits, (uint8_t) shift) ;
					}
				}

				// un-mix the data and convert to output format
				// - note that mixRes = 0 means just interleave so we use that path for uncompressed frames
				switch (p->mConfig.bitDepth)
				{
					case 16:
						out32 = sampleBuffer + channelIndex ;
						unmix16 (p->mMixBufferU, p->mMixBufferV, out32, numChannels, numSamples, mixBits, mixResiduals) ;
						break ;
					case 20:
						out32 = sampleBuffer + channelIndex ;
						unmix20 (p->mMixBufferU, p->mMixBufferV, out32, numChannels, numSamples, mixBits, mixResiduals) ;
						break ;
					case 24:
						out32 = sampleBuffer + channelIndex ;
						unmix24 (p->mMixBufferU, p->mMixBufferV, out32, numChannels, numSamples,
									mixBits, mixResiduals, p->u.mShiftBuffer, bytesShifted) ;
						break ;
					case 32:
						out32 = sampleBuffer + channelIndex ;
						unmix32 (p->mMixBufferU, p->mMixBufferV, out32, numChannels, numSamples,
									mixBits, mixResiduals, p->u.mShiftBuffer, bytesShifted) ;
						break ;
				}

				channelIndex += 2 ;
				*outNumSamples = numSamples ;
				break ;
			}

			case ID_CCE:
			case ID_PCE:
			{
				// unsupported element, bail
				//AssertNoErr (tag) ;
				status = kALAC_UnsupportedElement ;
				break ;
			}

			case ID_DSE:
			{
				// data stream element -- parse but ignore
				status = alac_data_stream_element (bits) ;
				break ;
			}

			case ID_FIL:
			{
				// fill element -- parse but ignore
				status = alac_fill_element (bits) ;
				break ;
			}

			case ID_END:
			{
				// frame end, all done so byte align the frame and check for overruns
				BitBufferByteAlign (bits, false) ;
				//Assert (bits->cur == bits->end) ;
				goto Exit ;
			}
		}

#if 1 // ! DEBUG
		// if we've decoded all of our channels, bail (but not in debug b/c we want to know if we're seeing bad bits)
		// - this also protects us if the config does not match the bitstream or crap data bits follow the audio bits
		if (channelIndex >= numChannels)
			break ;
#endif