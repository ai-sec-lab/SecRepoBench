
		/*file:///C:\ scheme*/
		if ((strlen(sURL)>=10) && (sURL[7]=='/') && (sURL[9]==':')) {
			memmove(sURL, sURL+8, strlen(sURL)-7);
		} else {
			memmove(sURL, sURL+7, strlen(sURL)-6);
		}
	