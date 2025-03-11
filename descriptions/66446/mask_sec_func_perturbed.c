int
xmlAddIDSafe(xmlDocPtr doc, const xmlChar *identifier, xmlAttrPtr attr,
             int streaming, xmlIDPtr *id) {
    xmlIDPtr ret;
    xmlIDTablePtr table;

    if (id != NULL)
        *id = NULL;

    if (doc == NULL) {
	return(-1);
    }
    if ((identifier == NULL) || (identifier[0] == 0)) {
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
        ret = xmlHashLookup(table, identifier);
        if (ret != NULL) {
            /*
             * Update the attribute to make entities work.
             */
            if (ret->attr != NULL) {
                ret->attr->id = NULL;
                ret->attr = attr;
            }
	    attr->atype = XML_ATTRIBUTE_ID;
            attr->id = ret;
            return(0);
        }
    }

    ret = (xmlIDPtr) xmlMalloc(sizeof(xmlID));
    if (ret == NULL)
	return(-1);
    memset(ret, 0, sizeof(*ret));

    /*
     * fill the structure.
     */
    ret->doc = doc;
    ret->value = xmlStrdup(identifier);
    if (ret->value == NULL) {
        xmlFreeID(ret);
        return(-1);
    }
    // <MASK>
}