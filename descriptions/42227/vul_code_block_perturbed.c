int sng;

        sng = it8_parser->ch;
        StringClear(it8_parser->str);

        NextCh(it8_parser);

        while (it8_parser->ch != sng) {

            if (it8_parser->ch == '\n' || it8_parser->ch == '\r') break;
            else {
                StringAppend(it8_parser->str, (char)it8_parser->ch);
                NextCh(it8_parser);
            }
        }

        it8_parser->sy = SSTRING;
        NextCh(it8_parser);