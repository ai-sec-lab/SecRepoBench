if (ret == NULL) {
            ctxt->error = XPATH_MEMORY_ERROR;
            xmlXPathFreeObject(obj1);
            xmlXPathFreeObject(obj2);
            return;
        }

	if (obj2->nodesetval != NULL) {
	    for (i = 0; i < obj2->nodesetval->nodeNr; i++) {
		valuePush(ctxt, xmlXPathObjectCopy(obj1));
		valuePush(ctxt,
			  xmlXPathNewNodeSet(obj2->nodesetval->nodeTab[i]));
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