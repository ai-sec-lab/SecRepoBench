static int
xmlTextReaderRemoveID(xmlDocPtr doc, xmlAttrPtr attr) {
    xmlIDTablePtr table;
    xmlIDPtr id;
    xmlChar *ID;

    if (doc == NULL) return(-1);
    if (attr == NULL) return(-1);
    table = (xmlIDTablePtr) doc->ids;
    // <MASK>
}