if (ret->attr != NULL) {
                ret->attr->id = NULL;
                ret->attr = attr;
            }
	    attr->atype = XML_ATTRIBUTE_ID;
            attr->id = ret;
            return(0);