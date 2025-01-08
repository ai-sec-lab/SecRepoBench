if (it8->ch == '\n' || it8->ch == '\r' || it8->ch == 0) break;
            else {
                StringAppend(it8->str, (char)it8->ch);
                NextCh(it8);
            }