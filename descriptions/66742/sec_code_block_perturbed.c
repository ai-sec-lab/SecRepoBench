if (test[i]=='[') is_ipv6 = GF_TRUE;
		else if (test[i]==']') is_ipv6 = GF_FALSE;
		if ( (test[i] == '/') || (!is_ipv6 && (test[i] == ':')) ) break;
		if (i>=GF_ARRAY_LENGTH(text)) break;
		text[i] = test[i];
		i += 1;