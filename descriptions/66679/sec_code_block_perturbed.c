if (ret->attr != NULL) {
                ret->attr->id = NULL;
                ret->attr = attribute;
                attribute->id = ret;
            }
            ret->lineno = xmlGetLineNo(attribute->parent);
	    attribute->atype = XML_ATTRIBUTE_ID;
            return(0);