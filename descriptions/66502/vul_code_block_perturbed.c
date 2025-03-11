ret = xmlHashLookup(table, value);
        if (ret != NULL) {
            /*
             * Update the attribute to make entities work.
             */
            if (ret->attr != NULL) {
                ret->attr->id = NULL;
                ret->attr = attribute;
            }
	    attribute->atype = XML_ATTRIBUTE_ID;
            attribute->id = ret;
            return(0);
        }