ret = xmlNewCDataBlock(ctxt->myDoc, cdataContent, len);
	xmlAddChild(ctxt->node, ret);