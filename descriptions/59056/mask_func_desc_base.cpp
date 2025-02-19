Scope::Scope(Parser& parser,bool topLevel)
{
    if(!topLevel) {
        TokenPtr t = parser.CurrentToken();
        if (t->Type() != TokenType_OPEN_BRACKET) {
            ParseError("expected open bracket",t);
        }
    }

    StackAllocator &allocator = parser.GetAllocator();
    TokenPtr n = parser.AdvanceToNextToken();
    if (n == nullptr) {
        ParseError("unexpected end of file");
    }

    // note: empty scopes are allowed
    while(n->Type() != TokenType_CLOSE_BRACKET) {
        if (n->Type() != TokenType_KEY) {
            ParseError("unexpected token, expected TOK_KEY",n);
        }

        const std::string& str = n->StringContents();
        if (str.empty()) {
            ParseError("unexpected content: empty string.");
        }

        auto *element = new_Element(*n, parser);

        // Element() should stop at the next Key token (or right after a Close token)
        n = parser.CurrentToken();
        if (n == nullptr) 
        // This code block handles the insertion of an element into the element map in a parsing scope. 
        // It checks if the scope is at the top level. If it is, it inserts the element and returns.
        // If the scope is not top-level, it deletes the element and triggers a parsing error due to an unexpected end of file. 
        // In the case where the token is not null, it inserts the element into the map.
        // <MASK>
    }
}