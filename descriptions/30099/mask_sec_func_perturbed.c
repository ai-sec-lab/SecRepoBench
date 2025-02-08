static void
xsltPreprocessStylesheet(xsltStylesheetPtr style, xmlNodePtr cur)
{
    xmlNodePtr deleteNode, styleelem;
    int internalize = 0;

    if ((style == NULL) || (cur == NULL))
        return;

    if ((cur->doc != NULL) && (style->dict != NULL) &&
        (cur->doc->dict == style->dict))
	internalize = 1;
    else
        style->internalized = 0;

    if ((cur != NULL) && (IS_XSLT_ELEM(cur)) &&
        (IS_XSLT_NAME(cur, "stylesheet"))) {
	styleelem = cur;
    } else {
        styleelem = NULL;
    }

    /*
     * This content comes from the stylesheet
     * For stylesheets, the set of whitespace-preserving
     * element names consists of just xsl:text.
     */
    deleteNode = NULL;
    while (cur != NULL) {
	if (deleteNode != NULL) {
#ifdef WITH_XSLT_DEBUG_BLANKS
	    xsltGenericDebug(xsltGenericDebugContext,
	     "xsltPreprocessStylesheet: removing ignorable blank node\n");
#endif
	    xmlUnlinkNode(deleteNode);
	    xmlFreeNode(deleteNode);
	    deleteNode = NULL;
	}
	if (cur->type == XML_ELEMENT_NODE) {
	    int excludedPrefixes;
	    /*
	     * Internalize attributes values.
	     */
	    if ((internalize) && (cur->properties != NULL)) {
	        xmlAttrPtr attr = cur->properties;
		xmlNodePtr txt;

		while (attr != NULL) {
		    txt = attr->children;
		    if ((txt != NULL) && (txt->type == XML_TEXT_NODE) &&
		        (txt->content != NULL) &&
			(!xmlDictOwns(style->dict, txt->content)))
		    {
			xmlChar *tmp;

			/*
			 * internalize the text string, goal is to speed
			 * up operations and minimize used space by compiled
			 * stylesheets.
			 */
			tmp = (xmlChar *) xmlDictLookup(style->dict,
			                                txt->content, -1);
			if (tmp != txt->content) {
			    xmlNodeSetContent(txt, NULL);
			    txt->content = tmp;
			}
		    }
		    attr = attr->next;
		}
	    }
	    if (IS_XSLT_ELEM(cur)) {
		excludedPrefixes = 0;
		if (IS_XSLT_NAME(cur, "text")) {
		    for (;excludedPrefixes > 0;excludedPrefixes--)
			exclPrefixPop(style);
		    goto skip_children;
		}
	    } else {
		excludedPrefixes = xsltParseStylesheetExcludePrefix(style, cur, 0);
	    }

	    if ((cur->nsDef != NULL) && (style->exclPrefixNr > 0)) {
		xmlNsPtr ns = cur->nsDef, prev = NULL, next;
		xmlNodePtr root = NULL;
		int i, moved;

		root = xmlDocGetRootElement(cur->doc);
		if ((root != NULL) && (root != cur)) {
		    while (ns != NULL) {
			moved = 0;
			next = ns->next;
			for (i = 0;i < style->exclPrefixNr;i++) {
			    if ((ns->prefix != NULL) &&
			        (xmlStrEqual(ns->href,
					     style->exclPrefixTab[i]))) {
				/*
				 * Move the namespace definition on the root
				 * element to avoid duplicating it without
				 * loosing it.
				 */
				if (prev == NULL) {
				    cur->nsDef = ns->next;
				} else {
				    prev->next = ns->next;
				}
				ns->next = root->nsDef;
				root->nsDef = ns;
				moved = 1;
				break;
			    }
			}
			if (moved == 0)
			    prev = ns;
			ns = next;
		    }
		}
	    }
	    /*
	     * If we have prefixes locally, recurse and pop them up when
	     * going back
	     */
	    if (excludedPrefixes > 0) {
		xsltPreprocessStylesheet(style, cur->children);
		for (;excludedPrefixes > 0;excludedPrefixes--)
		    exclPrefixPop(style);
		goto skip_children;
	    }
	} else if (cur->type == XML_TEXT_NODE) {
	    if (IS_BLANK_NODE(cur)) {
		if (xmlNodeGetSpacePreserve(cur->parent) != 1) {
		    deleteNode = cur;
		}
	    } else if ((cur->content != NULL) && (internalize) &&
	               (!xmlDictOwns(style->dict, cur->content))) {
		xmlChar *tmp;

		/*
		 * internalize the text string, goal is to speed
		 * up operations and minimize used space by compiled
		 * stylesheets.
		 */
		tmp = (xmlChar *) xmlDictLookup(style->dict, cur->content, -1);
		xmlNodeSetContent(cur, NULL);
		cur->content = tmp;
	    }
	} else if ((cur->type != XML_ELEMENT_NODE) &&
		   (cur->type != XML_CDATA_SECTION_NODE)) {
	    deleteNode = cur;
	    goto skip_children;
	}

	/*
	 * Skip to next node. In case of a namespaced element children of
	 * the stylesheet and not in the XSLT namespace and not an extension
	 * element, ignore its content.
	 */
	if ((cur->type == XML_ELEMENT_NODE) && (cur->ns != NULL) &&
	    (styleelem != NULL) && (cur->parent == styleelem) &&
	    (!xmlStrEqual(cur->ns->href, XSLT_NAMESPACE)) &&
	    (!xsltCheckExtURI(style, cur->ns->href))) {
	    goto skip_children;
	} else if (cur->children != NULL) {
	    // <MASK>
	}

skip_children:
	if (cur->next != NULL) {
	    cur = cur->next;
	    continue;
	}
	do {

	    cur = cur->parent;
	    if (cur == NULL)
		break;
	    if (cur == (xmlNodePtr) style->doc) {
		cur = NULL;
		break;
	    }
	    if (cur->next != NULL) {
		cur = cur->next;
		break;
	    }
	} while (cur != NULL);
    }
    if (deleteNode != NULL) {
#ifdef WITH_XSLT_DEBUG_PARSING
	xsltGenericDebug(xsltGenericDebugContext,
	 "xsltPreprocessStylesheet: removing ignorable blank node\n");
#endif
	xmlUnlinkNode(deleteNode);
	xmlFreeNode(deleteNode);
    }
}