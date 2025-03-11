static
void InStringSymbol(cmsIT8* it8_parser)
{
    while (isseparator(it8_parser->ch))
        NextCh(it8_parser);

    if (it8_parser->ch == '\'' || it8_parser->ch == '\"')
    {
        // <MASK>        
    }
    else
        SynError(it8_parser, "String expected");

}