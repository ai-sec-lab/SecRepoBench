{
            /*
             * Update the attribute unless we are parsing in streaming
             * mode. If the attribute is copied from an entity we want
             * the ID reference the copy.
             */
            if (ret->attr != NULL) {
                ret->attr->id = NULL;
                ret->attr = attribute;
                attribute->id = ret;
            }
            ret->lineno = xmlGetLineNo(attribute->parent);
	    attribute->atype = XML_ATTRIBUTE_ID;
            return(0);
        }