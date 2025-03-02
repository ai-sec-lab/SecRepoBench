static void
xmlTextReaderFreeProp(xmlTextReaderPtr reader, xmlAttrPtr cur) {
    xmlDictPtr dict;

    if ((reader != NULL) && (reader->ctxt != NULL))
	dict = reader->ctxt->dict;
    else
        dict = NULL;
    if (cur == NULL) return;

    // <MASK>
    if ((reader != NULL) && (reader->ctxt != NULL) &&
        (reader->ctxt->freeAttrsNr < 100)) {
        cur->next = reader->ctxt->freeAttrs;
	reader->ctxt->freeAttrs = cur;
	reader->ctxt->freeAttrsNr++;
    } else {
	xmlFree(cur);
    }
}