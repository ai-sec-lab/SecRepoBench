if (id == NULL || id->attr != attr) {
	return(-1);
    }
    id->name = attr->name;
    attr->name = NULL;
    id->attr = NULL;
    return(0);