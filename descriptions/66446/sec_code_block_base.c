if (streaming) {
	/*
	 * Operating in streaming mode, attr is gonna disappear
	 */
	if (doc->dict != NULL)
	    ret->name = xmlDictLookup(doc->dict, attr->name, -1);
	else
	    ret->name = xmlStrdup(attr->name);
        if (ret->name == NULL) {
            xmlFreeID(ret);
            return(-1);
        }
	ret->attr = NULL;
    } else {
	ret->attr = attr;
	ret->name = NULL;
    }
    ret->lineno = xmlGetLineNo(attr->parent);

    if (xmlHashAddEntry(table, value, ret) < 0) {
	xmlFreeID(ret);
	return(-1);
    }

    attr->atype = XML_ATTRIBUTE_ID;
    if (!streaming)
        attr->id = ret;

    if (id != NULL)
        *id = ret;
    return(1);