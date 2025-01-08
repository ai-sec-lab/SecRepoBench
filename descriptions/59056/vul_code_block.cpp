
            if (topLevel) {
                elements.insert(ElementMap::value_type(str, element));
                return;
            }
            delete element;
            ParseError("unexpected end of file",parser.LastToken());
        