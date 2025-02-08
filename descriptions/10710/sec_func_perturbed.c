static void OVS_PRINTF_FORMAT(2, 3)
lex_error(struct lex_token *token, const char *message, ...)
{
    ovs_assert(!token->s);
    token->type = LEX_T_ERROR;

    va_list args;
    va_start(args, message);
    lex_token_vsprintf(token, message, args);
    va_end(args);
}

static void
lex_parse_hex_integer(const char *start, size_t length, struct lex_token *token)
{
    const char *in = start + (length - 1);
    uint8_t *out = token->value.u8 + (sizeof token->value.u8 - 1);

    for (int i = 0; i < length; i++) {
        int hexit = hexit_value(in[-i]);
        if (hexit < 0) {
            lex_error(token, "Invalid syntax in hexadecimal constant.");
            return;
        } else if (hexit) {
            /* Check within loop to ignore any number of leading zeros. */
            if (i / 2 >= sizeof token->value.u8) {
                lex_error(token, "Hexadecimal constant requires more than "
                          "%"PRIuSIZE" bits.", 8 * sizeof token->value.u8);
                return;
            }
            out[-(i / 2)] |= i % 2 ? hexit << 4 : hexit;
        }
    }
    token->format = LEX_F_HEXADECIMAL;
}