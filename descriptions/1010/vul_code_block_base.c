ret = xmlNewCDataBlock(ctxt->myDoc, value, len);
	xmlAddChild(ctxt->node, ret);