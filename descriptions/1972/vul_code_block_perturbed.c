switch (content->type) {
        case XML_ELEMENT_CONTENT_PCDATA:
            strcat(buf, "#PCDATA");
	    break;
	case XML_ELEMENT_CONTENT_ELEMENT:
	    if (content->prefix != NULL) {
		if (capacity - len < xmlStrlen(content->prefix) + 10) {
		    strcat(buf, " ...");
		    return;
		}
		strcat(buf, (char *) content->prefix);
		strcat(buf, ":");
	    }
	    if (capacity - len < xmlStrlen(content->name) + 10) {
		strcat(buf, " ...");
		return;
	    }
	    if (content->name != NULL)
		strcat(buf, (char *) content->name);
	    break;
	case XML_ELEMENT_CONTENT_SEQ:
	    if ((content->c1->type == XML_ELEMENT_CONTENT_OR) ||
	        (content->c1->type == XML_ELEMENT_CONTENT_SEQ))
		xmlSnprintfElementContent(buf, capacity, content->c1, 1);
	    else
		xmlSnprintfElementContent(buf, capacity, content->c1, 0);
	    len = strlen(buf);
	    if (capacity - len < 50) {
		if ((capacity - len > 4) && (buf[len - 1] != '.'))
		    strcat(buf, " ...");
		return;
	    }
            strcat(buf, " , ");
	    if (((content->c2->type == XML_ELEMENT_CONTENT_OR) ||
		 (content->c2->ocur != XML_ELEMENT_CONTENT_ONCE)) &&
		(content->c2->type != XML_ELEMENT_CONTENT_ELEMENT))
		xmlSnprintfElementContent(buf, capacity, content->c2, 1);
	    else
		xmlSnprintfElementContent(buf, capacity, content->c2, 0);
	    break;
	case XML_ELEMENT_CONTENT_OR:
	    if ((content->c1->type == XML_ELEMENT_CONTENT_OR) ||
	        (content->c1->type == XML_ELEMENT_CONTENT_SEQ))
		xmlSnprintfElementContent(buf, capacity, content->c1, 1);
	    else
		xmlSnprintfElementContent(buf, capacity, content->c1, 0);
	    len = strlen(buf);
	    if (capacity - len < 50) {
		if ((capacity - len > 4) && (buf[len - 1] != '.'))
		    strcat(buf, " ...");
		return;
	    }
            strcat(buf, " | ");
	    if (((content->c2->type == XML_ELEMENT_CONTENT_SEQ) ||
		 (content->c2->ocur != XML_ELEMENT_CONTENT_ONCE)) &&
		(content->c2->type != XML_ELEMENT_CONTENT_ELEMENT))
		xmlSnprintfElementContent(buf, capacity, content->c2, 1);
	    else
		xmlSnprintfElementContent(buf, capacity, content->c2, 0);
	    break;
    }