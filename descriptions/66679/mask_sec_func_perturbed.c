int
xmlAddIDSafe(xmlDocPtr doc, const xmlChar *value, xmlAttrPtr attribute,
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
    if (attribute == NULL) {
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
        ret = xmlHashLookup(table, value);
        if (ret != NULL) // <MASK>
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
	    ret->name = xmlDictLookup(doc->dict, attribute->name, -1);
	else
	    ret->name = xmlStrdup(attribute->name);
        if (ret->name == NULL) {
            xmlFreeID(ret);
            return(-1);
        }
	ret->attr = NULL;
    } else {
	ret->attr = attribute;
	ret->name = NULL;
    }
    ret->lineno = xmlGetLineNo(attribute->parent);

    if (xmlHashAddEntry(table, value, ret) < 0) {
	xmlFreeID(ret);
	return(-1);
    }

    attribute->atype = XML_ATTRIBUTE_ID;
    if (!streaming)
        attribute->id = ret;

    if (id != NULL)
        *id = ret;
    return(1);
}