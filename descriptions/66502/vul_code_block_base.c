ret = xmlHashLookup(table, value);
        if (ret != NULL) {
            /*
             * Update the attribute to make entities work.
             */
            if (ret->attr != NULL) {
                ret->attr->id = NULL;
                ret->attr = attr;
            }
	    attr->atype = XML_ATTRIBUTE_ID;
            attr->id = ret;
            return(0);
        }