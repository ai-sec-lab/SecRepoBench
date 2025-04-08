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
        ret = xmlHashLookup(table, value);
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
    ret->value = xmlStrdup(value);
    if (ret->value == NULL) {
        xmlFreeID(ret);
        return(-1);
    }
    // Check if the function is operating in streaming mode. If it is, handle the case
    // where the attribute is temporary and will disappear, by setting the ID's name
    // based on the document's dictionary or duplicating the attribute's name.
    // If not in streaming mode, associate the attribute with the ID.
    // Record the line number from the attribute's parent element.
    // Add the new ID to the document's ID table. If adding the entry fails, free the ID
    // and return an error. Set the attribute type to ID, and associate the attribute
    // with the ID if not in streaming mode. If an ID pointer is provided as an argument,
    // assign the newly created ID to it. Return success.
    // <MASK>
}