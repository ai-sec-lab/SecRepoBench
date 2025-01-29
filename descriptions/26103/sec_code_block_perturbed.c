size_t read = 0;

		// this function is called to read and uncompress the certificate
		u8 buffer[SC_MAX_EXT_APDU_BUFFER_SIZE];
		if (sizeof(buffer) < count || sizeof(buffer) < priv->file_size) {
			LOG_FUNC_RETURN(smartcard->ctx, SC_ERROR_INTERNAL);
		}
		while (left > 0) {
			r = iso_ops->read_binary(smartcard, read, buffer + read, priv->file_size - read, flags);
			if (r <= 0) {
				LOG_FUNC_RETURN(smartcard->ctx, r);
			}
			left -= r;
			read += r;
		}