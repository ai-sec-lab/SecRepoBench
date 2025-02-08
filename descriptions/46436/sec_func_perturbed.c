static gboolean
tag_is_ascii( ExifTag exiftag )
{
	return( exiftag == EXIF_TAG_MAKE ||
		exiftag == EXIF_TAG_MODEL ||
		exiftag == EXIF_TAG_IMAGE_DESCRIPTION ||
		exiftag == EXIF_TAG_ARTIST ||
		exiftag == EXIF_TAG_SOFTWARE ||
		exiftag == EXIF_TAG_COPYRIGHT ||
		exiftag == EXIF_TAG_DATE_TIME ||
		exiftag == EXIF_TAG_DATE_TIME_ORIGINAL ||
		exiftag == EXIF_TAG_DATE_TIME_DIGITIZED ||
		exiftag == EXIF_TAG_SUB_SEC_TIME ||
		exiftag == EXIF_TAG_SUB_SEC_TIME_ORIGINAL ||
		exiftag == EXIF_TAG_SUB_SEC_TIME_DIGITIZED
#ifdef HAVE_EXIF_0_6_22
		|| exiftag == EXIF_TAG_CAMERA_OWNER_NAME
		|| exiftag == EXIF_TAG_BODY_SERIAL_NUMBER
		|| exiftag == EXIF_TAG_LENS_MAKE
		|| exiftag == EXIF_TAG_LENS_MODEL
		|| exiftag == EXIF_TAG_LENS_SERIAL_NUMBER
#endif
#ifdef HAVE_EXIF_0_6_23
		|| exiftag == EXIF_TAG_OFFSET_TIME
		|| exiftag == EXIF_TAG_OFFSET_TIME_ORIGINAL
		|| exiftag == EXIF_TAG_OFFSET_TIME_DIGITIZED
#endif
		);
}