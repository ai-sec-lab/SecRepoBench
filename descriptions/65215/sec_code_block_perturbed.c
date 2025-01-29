if ((strlen(urlString)>=10) && (urlString[7]=='/') && (urlString[9]==':')) {
			memmove(urlString, urlString+8, strlen(urlString)-7);
		} else {
			memmove(urlString, urlString+7, strlen(urlString)-6);
		}