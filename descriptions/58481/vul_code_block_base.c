for(channel = 0; channel < frame->header.channels; channel++)
					newbuffer[channel] = buffer[channel] + delta;
				decoder->private_->last_frame.header.blocksize -= delta;
				decoder->private_->last_frame.header.number.sample_number += (FLAC__uint64)delta;
				/* write the relevant samples */
				return decoder->private_->write_callback(decoder, &decoder->private_->last_frame, newbuffer, decoder->private_->client_data);