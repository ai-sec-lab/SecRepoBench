static void
xmlParseConditionalSections(xmlParserCtxtPtr ctxt) {
    int *inputIds = NULL;
    size_t inputIdsSize = 0;
    size_t depth = 0;

    while (PARSER_STOPPED(ctxt) == 0) {
        if ((RAW == '<') && (NXT(1) == '!') && (NXT(2) == '[')) {
            int id = ctxt->input->id;

            SKIP(3);
            SKIP_BLANKS;

            if (CMP7(CUR_PTR, 'I', 'N', 'C', 'L', 'U', 'D', 'E')) {
                SKIP(7);
                SKIP_BLANKS;
                if (RAW != '[') {
                    xmlFatalErr(ctxt, XML_ERR_CONDSEC_INVALID, NULL);
                    xmlHaltParser(ctxt);
                    goto error;
                }
                if (ctxt->input->id != id) {
                    xmlFatalErrMsg(ctxt, XML_ERR_ENTITY_BOUNDARY,
                                   "All markup of the conditional section is"
                                   " not in the same entity\n");
                }
                NEXT;

                if (inputIdsSize <= depth) {
                    int *tmp;

                    inputIdsSize = (inputIdsSize == 0 ? 4 : inputIdsSize * 2);
                    tmp = (int *) xmlRealloc(inputIds,
                            inputIdsSize * sizeof(int));
                    if (tmp == NULL) {
                        xmlErrMemory(ctxt, NULL);
                        goto error;
                    }
                    inputIds = tmp;
                }
                inputIds[depth] = id;
                depth++;
            } else if (CMP6(CUR_PTR, 'I', 'G', 'N', 'O', 'R', 'E')) // <MASK>
        } else if ((depth > 0) &&
                   (RAW == ']') && (NXT(1) == ']') && (NXT(2) == '>')) {
            depth--;
            if (ctxt->input->id != inputIds[depth]) {
                xmlFatalErrMsg(ctxt, XML_ERR_ENTITY_BOUNDARY,
                               "All markup of the conditional section is not"
                               " in the same entity\n");
            }
            SKIP(3);
        } else if ((RAW == '<') && ((NXT(1) == '!') || (NXT(1) == '?'))) {
            xmlParseMarkupDecl(ctxt);
        } else {
            xmlFatalErr(ctxt, XML_ERR_EXT_SUBSET_NOT_FINISHED, NULL);
            xmlHaltParser(ctxt);
            goto error;
        }

        if (depth == 0)
            break;

        SKIP_BLANKS;
        SHRINK;
        GROW;
    }

error:
    xmlFree(inputIds);
}