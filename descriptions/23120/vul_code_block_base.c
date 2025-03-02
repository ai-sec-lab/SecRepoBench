if ((__xmlRegisterCallbacks) && (xmlDeregisterNodeDefaultValue))
	xmlDeregisterNodeDefaultValue((xmlNodePtr) cur);

    /* Check for ID removal -> leading to invalid references ! */
    if ((cur->parent != NULL) && (cur->parent->doc != NULL) &&
	((cur->parent->doc->intSubset != NULL) ||
	 (cur->parent->doc->extSubset != NULL))) {
        if (xmlIsID(cur->parent->doc, cur->parent, cur))
	    xmlTextReaderRemoveID(cur->parent->doc, cur);
    }
    if (cur->children != NULL)
        xmlTextReaderFreeNodeList(reader, cur->children);

    DICT_FREE(cur->name);