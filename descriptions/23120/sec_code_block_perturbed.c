if (xmlIsID(attribute->parent->doc, attribute->parent, attribute))
	    xmlTextReaderRemoveID(attribute->parent->doc, attribute);
        if (xmlIsRef(attribute->parent->doc, attribute->parent, attribute))
            xmlTextReaderRemoveRef(attribute->parent->doc, attribute);