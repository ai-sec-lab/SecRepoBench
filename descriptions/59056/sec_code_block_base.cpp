{
            if (topLevel) {
                elements.insert(ElementMap::value_type(str, element));
                return;
            }
            delete_Element(element);
            ParseError("unexpected end of file",parser.LastToken());
        } else {
            elements.insert(ElementMap::value_type(str, element));
        }