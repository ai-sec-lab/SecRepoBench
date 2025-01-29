++p;

        goto match_post_value;

    match_array:
        SkASSERT(*p == '[');
        p = skip_ws(p + 1);

        this->pushArrayScope();

        if (*p != ']') goto match_value;

        // goto pop_array;
    pop_array:
        SkASSERT(*p == ']');

        if (fScopeStack.back() >= 0) {
            return this->error(NullValue(), p, "unexpected array terminator");
        }

        this->popArrayScope();

        goto pop_common;

        SkASSERT(false);
        return NullValue();