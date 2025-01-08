if (size + 1 < remain-bytes_skipped) {
			//make sure we are sync!
			if (sync[size] !=0xFF) {
				if ((sync[size]=='T') && (sync[size+1]=='A') && (sync[size+2]=='G')) {
					skip_id3v1=GF_TRUE;
				} else {
					GF_LOG(ctx->is_sync ? GF_LOG_WARNING : GF_LOG_DEBUG, GF_LOG_MEDIA, ("[MP3Dmx] invalid frame, resyncing\n"));
					ctx->is_sync = GF_FALSE;
					goto drop_byte;
				}
			}
		}
		//otherwise wait for next frame, unless if end of stream
		else if (pck) {
			break;
		}