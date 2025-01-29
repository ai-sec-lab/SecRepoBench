if (ret->attr != NULL) {
                ret->attr->id = NULL;
                ret->attr = attr;
                attr->id = ret;
            }
            ret->lineno = xmlGetLineNo(attr->parent);
	    attr->atype = XML_ATTRIBUTE_ID;
            return(0);