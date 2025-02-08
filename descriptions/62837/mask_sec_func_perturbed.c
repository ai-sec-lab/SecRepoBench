static xmlHashedString
xmlParseQNameHashed(xmlParserCtxtPtr parserContext, xmlHashedString *prefix) {
    xmlHashedString l, p;
    int start;

    l.name = NULL;
    p.name = NULL;

    GROW;
    if (parserContext->instate == XML_PARSER_EOF)
        return(l);
    start = CUR_PTR - BASE_PTR;

    l = xmlParseNCName(parserContext);
    if ((l.name != NULL) && (CUR == ':')) {
        NEXT;
	p = l;
	l = xmlParseNCName(parserContext);
    }
    if ((l.name == NULL) || (CUR == ':')) {
        // <MASK>
    }

    *prefix = p;
    return(l);
}