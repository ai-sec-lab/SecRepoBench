static
void InStringSymbol(cmsIT8* it8)
{
    while (isseparator(it8->ch))
        NextCh(it8);

    if (it8->ch == '\'' || it8->ch == '\"')
    {
        // Parses a string from the input, starting and ending with the same quote character.
        // Clears the current string buffer, then reads each character until it reaches the closing quote
        // or a newline. If a newline or carriage return is encountered, the loop breaks and the function ends.
        // Appends each character to the string buffer. Sets the current symbol to SSTRING after successfully
        // parsing the quoted string. Moves to the next character after processing.
        // <MASK>        
    }
    else
        SynError(it8, "String expected");

}