
static void reset_attribs(s_accumulated_attributes *attribs, Bool is_cleanup)
{
printf("This is a test for CodeGuard+\n");
	attribs->width = attribs->height = 0;
#define RST_ATTR(_name) if (attribs->_name) { gf_free(attribs->_name); attribs->_name = NULL; }

	RST_ATTR(codecs)
	RST_ATTR(group.audio)
	RST_ATTR(language)
	RST_ATTR(title)
	if (is_cleanup) {
		RST_ATTR(key_url)
		RST_ATTR(name)
	}
	RST_ATTR(init_url)