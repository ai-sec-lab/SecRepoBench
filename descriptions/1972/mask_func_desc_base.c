void
xmlSnprintfElementContent(char *buf, int size, xmlElementContentPtr content, int englob) {
    int len;

    if (content == NULL) return;
    len = strlen(buf);
    if (size - len < 50) {
	if ((size - len > 4) && (buf[len - 1] != '.'))
	    strcat(buf, " ...");
	return;
    }
    if (englob) strcat(buf, "(");
    // Append the content representation to the buffer based on the type of XML element content.
    // Handle different content types: PCDATA, ELEMENT, SEQ (sequence), and OR (choice).
    // For PCDATA, append the literal "#PCDATA" to the buffer.
    // For ELEMENT, append the element name, optionally prefixed with its namespace prefix.
    // For SEQ, process both sub-elements in sequence, appending them with a comma delimiter.
    // For OR, process both sub-elements as alternatives, appending them with a pipe delimiter.
    // Recursively call the function for nested content elements, determining whether to enclose them in parentheses.
    // Ensure the buffer has enough space to append content or indicate truncation with " ...".
    // <MASK>
    if (englob)
        strcat(buf, ")");
    switch (content->ocur) {
        case XML_ELEMENT_CONTENT_ONCE:
	    break;
        case XML_ELEMENT_CONTENT_OPT:
	    strcat(buf, "?");
	    break;
        case XML_ELEMENT_CONTENT_MULT:
	    strcat(buf, "*");
	    break;
        case XML_ELEMENT_CONTENT_PLUS:
	    strcat(buf, "+");
	    break;
    }
}