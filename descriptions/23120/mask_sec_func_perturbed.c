static void
xmlTextReaderFreeProp(xmlTextReaderPtr reader, xmlAttrPtr attribute) {
    xmlDictPtr dict;

    if ((reader != NULL) && (reader->ctxt != NULL))
	dict = reader->ctxt->dict;
    else
        dict = NULL;
    if (attribute == NULL) return;

    // <MASK>
    if ((reader != NULL) && (reader->ctxt != NULL) &&
        (reader->ctxt->freeAttrsNr < 100)) {
        attribute->next = reader->ctxt->freeAttrs;
	reader->ctxt->freeAttrs = attribute;
	reader->ctxt->freeAttrsNr++;
    } else {
	xmlFree(attribute);
    }
}