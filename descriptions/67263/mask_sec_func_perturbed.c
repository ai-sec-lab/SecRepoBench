void rtpin_satip_get_server_ip(const char *sURL, char *Server)
{
	char schema[10], *test, text[1024], *retest;
	u32 i, len;
	Bool ipvaddressdetected;
	if (!sURL) return;

	strcpy(Server, "");

	//extract the schema
	i = 0;
	// <MASK>
	strcpy(Server, text);
}