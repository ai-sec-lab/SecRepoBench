if (csv_stream == NULL) {
									/* the enclosure is unterminated */
									if (bptr > limit) {
										/* if the line ends with enclosure, we need to go back by
										 * one character so the \0 character is not copied. */
										--bptr;
									}
									goto quit_loop_2;
								}

								size_t new_len;
								char *new_buf = php_stream_get_line(csv_stream, NULL, 0, &new_len);
								if (!new_buf) {
									/* we've got an unterminated enclosure,
									 * assign all the data from the start of
									 * the enclosure to end of data to the
									 * last element */
									if (bptr > limit) {
										/* if the line ends with enclosure, we need to go back by
										 * one character so the \0 character is not copied. */
										--bptr;
									}
									goto quit_loop_2;
								}