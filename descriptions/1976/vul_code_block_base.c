/* Validity Constraint: ID uniqueness */
    if (attrDecl->atype == XML_ATTRIBUTE_ID) {
        if (xmlAddID(ctxt, doc, value, (xmlAttrPtr) ns) == NULL)
	    ret = 0;
    }

    if ((attrDecl->atype == XML_ATTRIBUTE_IDREF) ||
	(attrDecl->atype == XML_ATTRIBUTE_IDREFS)) {
        if (xmlAddRef(ctxt, doc, value, (xmlAttrPtr) ns) == NULL)
	    ret = 0;
    }