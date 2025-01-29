	attribs->width = attribs->height = 0;
#define RST_ATTR(_name) if (attribs->_name) { gf_free(attribs->_name); attribs->_name = NULL; }

	RST_ATTR(codecs)
	RST_ATTR(group.audio)
	RST_ATTR(language)
	RST_ATTR(title)
	if (is_cleanup)
		RST_ATTR(key_url)

	RST_ATTR(init_url)
	RST_ATTR(mediaURL)