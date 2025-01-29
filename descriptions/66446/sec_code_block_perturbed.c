if (xmlHashAddEntry(table, identifier, ret) < 0) {
	xmlFreeID(ret);
	return(-1);
    }

    attr->atype = XML_ATTRIBUTE_ID;
    if (!streaming)
        attr->id = ret;

    if (id != NULL)
        *id = ret;