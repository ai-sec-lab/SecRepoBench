int
exif_mnote_data_apple_identify(const ExifData *exifdata, const ExifEntry *e) {
    if (e->size < strlen("Apple iOS")+1)
	return 0;

    return !memcmp((const char *) e->data, "Apple iOS", strlen("Apple iOS"));
}