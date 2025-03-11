static
void InStringSymbol(cmsIT8* it8_parser)
{
    while (isseparator(it8_parser->ch))
        NextCh(it8_parser);

    if (it8_parser->ch == '\'' || it8_parser->ch == '\"')
    {
        // Parses a string from the input, starting and ending with the same quote character.
        // Clears the current string buffer, then reads each character until it reaches the closing quote
        // or a newline. If a newline or carriage return is encountered, the loop breaks and the function ends.
        // Appends each character to the string buffer. Sets the current symbol to SSTRING after successfully
        // parsing the quoted string. Moves to the next character after processing.
        // <MASK>        
    }
    else
        SynError(it8_parser, "String expected");

}