/* Content in metadata block didn't fit in block length
				 * We cannot know whether the length or the content was
				 * corrupt, so stop parsing metadata */
				send_error_to_client_(decoder, FLAC__STREAM_DECODER_ERROR_STATUS_BAD_METADATA);
				if(decoder->protected_->state == FLAC__STREAM_DECODER_READ_METADATA)
					decoder->protected_->state = FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC;
				ok = false;