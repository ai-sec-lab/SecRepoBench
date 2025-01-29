GF_Err e;
				pic_size = (u32) ( (sep_desc + 1) - buf);
				pic_size = fsize - pic_size;

				if (video_pid_p) {
					e = gf_filter_pid_raw_new(filter, NULL, NULL, buf+1, NULL, sep_desc+1, pic_size, GF_FALSE, video_pid_p);
					if (e) {
						GF_LOG(GF_LOG_ERROR, GF_LOG_MEDIA, ("[MP3Dmx] error setting up video pid for cover art: %s\n", gf_error_to_string(e) ));
					}
					if (*video_pid_p) {
						u8 *out_buffer;
						GF_FilterPacket *dst_pck;
						gf_filter_pid_set_name(*video_pid_p, "CoverArt");
						gf_filter_pid_set_property(*video_pid_p, GF_PROP_PID_COVER_ART, &PROP_BOOL(GF_TRUE));
						dst_pck = gf_filter_pck_new_alloc(*video_pid_p, pic_size, &out_buffer);
						if (dst_pck) {
							gf_filter_pck_set_framing(dst_pck, GF_TRUE, GF_TRUE);
							memcpy(out_buffer, sep_desc+1, pic_size);
							gf_filter_pck_send(dst_pck);
						}

						gf_filter_pid_set_eos(*video_pid_p);
					}
				} else {
					gf_filter_pid_set_property(audio_pid, GF_PROP_PID_COVER_ART, &PROP_DATA(sep_desc+1, pic_size) );
				}