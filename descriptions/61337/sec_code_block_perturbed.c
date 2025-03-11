{
            handler = xmlFindCharEncodingHandler((const char *) encoding_guess);
            if (handler != NULL) {
                /*
                 * Don't use UTF-8 encoder which isn't required and
                 * can produce invalid UTF-8.
                 */
                if (!xmlStrEqual(BAD_CAST handler->name, BAD_CAST "UTF-8"))
                    xmlSwitchToEncoding(ctxt, handler);
            } else {
                htmlParseErr(ctxt, XML_ERR_INVALID_ENCODING,
                             "Unsupported encoding %s", encoding_guess, NULL);
            }
            xmlFree(encoding_guess);
        }