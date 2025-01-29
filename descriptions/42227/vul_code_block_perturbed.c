NextCh(it8);

        while (it8->ch != quotechar) {

            if (it8->ch == '\n' || it8->ch == '\r') break;
            else {
                StringAppend(it8->str, (char)it8->ch);
                NextCh(it8);
            }
        }

        it8->sy = SSTRING;
        NextCh(it8);