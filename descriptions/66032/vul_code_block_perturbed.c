probe_data = res;

	//strip all spaces and \r\n
	while (probe_data[0] && strchr("\n\r\t ", (char) probe_data[0]))
		probe_data ++;

	//for XML, strip doctype, <?xml and comments
	while (1) {
		char *search=NULL;
		if (!strncmp(probe_data, "<!DOCTYPE", 9)) {
			search = ">";
		}
		//for XML, strip xml header
		else if (!strncmp(probe_data, "<?xml", 5)) {
			search = "?>";
		}
		else if (!strncmp(probe_data, "<!--", 4)) {
			search = "-->";
		} else {
			break;
		}
		const char *res = my_strstr(probe_data, search, probelength);
		if (!res) goto exit;
		res += strlen(search);
		probelength -= (u32) (res - (char*)probe_data);
		probe_data = res;
		while (probelength && probe_data[0] && strchr("\n\r\t ", (char) probe_data[0])) {
			probe_data ++;
			probelength--;
		}
	}