if (e->size < strlen("Apple iOS")+1)
	return 0;

    return !memcmp((const char *) e->data, "Apple iOS", strlen("Apple iOS"));