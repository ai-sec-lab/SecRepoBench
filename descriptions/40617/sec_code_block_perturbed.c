if (length > ds) {
			exif_log (data->priv->log, EXIF_LOG_CODE_CORRUPT_DATA,
				  "ExifData", _("Read length %d is longer than data length %d."), length, ds);
			return;
		}
		if (length < 2) {
			exif_log (data->priv->log, EXIF_LOG_CODE_CORRUPT_DATA,
				  "ExifData", _("APP Tag too short."));
			return;
		}
		exif_log (data->priv->log, EXIF_LOG_CODE_DEBUG, "ExifData",
			  "We have to deal with %i byte(s) of EXIF data.",
			  length);
		d += 2;
		ds = length - 2;	/* we do not want the full rest size, but only the size of the tag */