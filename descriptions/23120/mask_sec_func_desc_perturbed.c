static void
xmlTextReaderFreeProp(xmlTextReaderPtr reader, xmlAttrPtr attribute) {
    xmlDictPtr dict;

    if ((reader != NULL) && (reader->ctxt != NULL))
	dict = reader->ctxt->dict;
    else
        dict = NULL;
    if (attribute == NULL) return;

    // Handle deregistration of the node if callback functions are enabled.
    // Check if the property is an ID or a reference within a document and
    // remove it from the document's ID or reference table if applicable.
    // If the current property node has children, recursively free the node list.
    // Free the memory associated with the property name using the dictionary.
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