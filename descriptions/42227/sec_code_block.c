
        int sng;

        sng = it8->ch;
        StringClear(it8->str);

        NextCh(it8);

        while (it8->ch != sng) {

            if (it8->ch == '\n' || it8->ch == '\r' || it8->ch == 0) break;
            else {
                StringAppend(it8->str, (char)it8->ch);
                NextCh(it8);
            }
        }

        it8->sy = SSTRING;
        NextCh(it8);        
    