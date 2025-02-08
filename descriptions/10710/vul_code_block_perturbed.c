int hexit = hexit_value(in[-i]);
        if (hexit < 0) {
            lex_error(token, "Invalid syntax in hexadecimal constant.");
            return;
        }
        if (hexit && i / 2 >= sizeof token->value.u8) {
            lex_error(token, "Hexadecimal constant requires more than "
                      "%"PRIuSIZE" bits.", 8 * sizeof token->value.u8);
            return;
        }
        out[-(i / 2)] |= i % 2 ? hexit << 4 : hexit;