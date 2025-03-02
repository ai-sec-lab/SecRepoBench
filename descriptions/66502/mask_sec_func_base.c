int
xmlAddIDSafe(xmlDocPtr doc, const xmlChar *value, xmlAttrPtr attr,
             int streaming, xmlIDPtr *id) {
    xmlIDPtr ret;
    xmlIDTablePtr table;

    if (id != NULL)
        *id = NULL;

    if (doc == NULL) {
	return(-1);
    }
    if ((value == NULL) || (value[0] == 0)) {
	return(0);
    }
    if (attr == NULL) {
	return(-1);
    }

    /*
     * Create the ID table if needed.
     */
    table = (xmlIDTablePtr) doc->ids;
    if (table == NULL)  {
        doc->ids = table = xmlHashCreateDict(0, doc->dict);
        if (table == NULL)
            return(-1);
    } else {
        // <MASK>
    }

    ret = (xmlIDPtr) xmlMalloc(sizeof(xmlID));
    if (ret == NULL)
	return(-1);
    memset(ret, 0, sizeof(*ret));

    /*
     * fill the structure.
     */
    ret->doc = doc;
    ret->value = xmlStrdup(value);
    if (ret->value == NULL) {
        xmlFreeID(ret);
        return(-1);
    }
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
}