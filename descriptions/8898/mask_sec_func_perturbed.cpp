const Value parse(const char* p, size_t length) {
        if (!length) {
            return this->error(NullValue(), p, "invalid empty input");
        }

        const char* p_stop = p + length - 1;

        // We're only checking for end-of-stream on object/array close('}',']'),
        // so we must trim any whitespace from the buffer tail.
        while (p_stop > p && is_ws(*p_stop)) --p_stop;

        SkASSERT(p_stop >= p && p_stop < p + length);
        if (*p_stop != '}' && *p_stop != ']') {
            return this->error(NullValue(), p_stop, "invalid top-level value");
        }

        p = skip_ws(p);

        switch (*p) {
        case '{':
            goto match_object;
        case '[':
            goto match_array;
        default:
            return this->error(NullValue(), p, "invalid top-level value");
        }

    match_object:
        SkASSERT(*p == '{');
        p = skip_ws(p + 1);

        this->pushObjectScope();

        if (*p == '}') goto pop_object;

        // goto match_object_key;
    match_object_key:
        p = skip_ws(p);
        if (*p != '"') return this->error(NullValue(), p, "expected object key");

        p = this->matchString(p, [this](const char* key, size_t length) {
            this->pushObjectKey(key, length);
        });
        if (!p) return NullValue();

        p = skip_ws(p);
        if (*p != ':') return this->error(NullValue(), p, "expected ':' separator");

        ++p;

        // goto match_value;
    match_value:
        p = skip_ws(p);

        switch (*p) {
        case '\0':
            return this->error(NullValue(), p, "unexpected input end");
        case '"':
            p = this->matchString(p, [this](const char* str, size_t length) {
                this->pushString(str, length);
            });
            break;
        case '[':
            goto match_array;
        case 'f':
            p = this->matchFalse(p);
            break;
        case 'n':
            p = this->matchNull(p);
            break;
        case 't':
            p = this->matchTrue(p);
            break;
        case '{':
            goto match_object;
        default:
            p = this->matchNumber(p);
            break;
        }

        if (!p) return NullValue();

        // goto match_post_value;
    match_post_value:
        SkASSERT(!fScopeStack.empty());

        p = skip_ws(p);
        switch (*p) {
        case ',':
            ++p;
            if (fScopeStack.back() >= 0) {
                goto match_object_key;
            } else {
                goto match_value;
            }
        case ']':
            goto pop_array;
        case '}':
            goto pop_object;
        default:
            return this->error(NullValue(), p - 1, "unexpected value-trailing token");
        }

        // unreachable
        SkASSERT(false);

    pop_object:
        SkASSERT(*p == '}');

        if (fScopeStack.back() < 0) {
            return this->error(NullValue(), p, "unexpected object terminator");
        }

        this->popObjectScope();

        // goto pop_common
    pop_common:
        SkASSERT(*p == '}' || *p == ']');

        if (fScopeStack.empty()) {
            SkASSERT(fValueStack.size() == 1);

            // Success condition: parsed the top level element and reached the stop token.
            return p == p_stop
                ? fValueStack.front()
                : this->error(NullValue(), p + 1, "trailing root garbage");
        }

        // <MASK>
    }