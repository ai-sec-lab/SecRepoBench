if (xmlIsID(cur->parent->doc, cur->parent, cur))
	    xmlTextReaderRemoveID(cur->parent->doc, cur);
        if (xmlIsRef(cur->parent->doc, cur->parent, cur))
            xmlTextReaderRemoveRef(cur->parent->doc, cur);