for(channel = 0; channel < frame->header.channels; channel++) {
					newbuffer[channel] = buffer[channel] + sampleoffset;
					decoder->private_->last_frame.subframes[channel].type = FLAC__SUBFRAME_TYPE_VERBATIM;
					decoder->private_->last_frame.subframes[channel].data.verbatim.data_type = FLAC__VERBATIM_SUBFRAME_DATA_TYPE_INT32;
					decoder->private_->last_frame.subframes[channel].data.verbatim.data.int32 = newbuffer[channel];
				}
				decoder->private_->last_frame.header.blocksize -= sampleoffset;
				decoder->private_->last_frame.header.number.sample_number += (FLAC__uint64)sampleoffset;
				/* write the relevant samples */
				return decoder->private_->write_callback(decoder, &decoder->private_->last_frame, newbuffer, decoder->private_->client_data);