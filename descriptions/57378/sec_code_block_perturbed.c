if (ret == NULL) {
            ctxt->error = XPATH_MEMORY_ERROR;
            xmlXPathFreeObject(obj1);
            xmlXPathFreeObject(keyValue);
            return;
        }

	if (keyValue->nodesetval != NULL) {
	    for (i = 0; i < keyValue->nodesetval->nodeNr; i++) {
		valuePush(ctxt, xmlXPathObjectCopy(obj1));
		valuePush(ctxt,
			  xmlXPathNewNodeSet(keyValue->nodesetval->nodeTab[i]));
		xmlXPathStringFunction(ctxt, 1);
		xsltKeyFunction(ctxt, 2);
		newobj = valuePop(ctxt);
                if (newobj != NULL)
		    ret->nodesetval = xmlXPathNodeSetMerge(ret->nodesetval,
						           newobj->nodesetval);
		xmlXPathFreeObject(newobj);
	    }
	}
	valuePush(ctxt, ret);