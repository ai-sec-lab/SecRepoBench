{
            /*
             * Update the attribute to make entities work.
             */
            if (!streaming) {
                if (ret->attr != NULL) {
                    ret->attr->id = NULL;
                    ret->attr = attribute;
                }
                attribute->id = ret;
            }
	    attribute->atype = XML_ATTRIBUTE_ID;
            return(0);
        }