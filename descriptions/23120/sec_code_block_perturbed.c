if ((__xmlRegisterCallbacks) && (xmlDeregisterNodeDefaultValue))
	xmlDeregisterNodeDefaultValue((xmlNodePtr) attribute);

    /* Check for ID removal -> leading to invalid references ! */
    if ((attribute->parent != NULL) && (attribute->parent->doc != NULL) &&
	((attribute->parent->doc->intSubset != NULL) ||
	 (attribute->parent->doc->extSubset != NULL))) {
        if (xmlIsID(attribute->parent->doc, attribute->parent, attribute))
	    xmlTextReaderRemoveID(attribute->parent->doc, attribute);
        if (xmlIsRef(attribute->parent->doc, attribute->parent, attribute))
            xmlTextReaderRemoveRef(attribute->parent->doc, attribute);
    }
    if (attribute->children != NULL)
        xmlTextReaderFreeNodeList(reader, attribute->children);

    DICT_FREE(attribute->name);