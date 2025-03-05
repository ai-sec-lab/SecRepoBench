static plist_t parse_primitive(const char* js, jsmntok_info_t* ti, int* index)
{
    if (ti->tokens[*index].type != JSMN_PRIMITIVE) {
        PLIST_JSON_ERR("%s: token type != JSMN_PRIMITIVE\n", __func__);
        return NULL;
    }
    plist_t val = NULL;
    const char* str_val = js + ti->tokens[*index].start;
    const char* str_end = js + ti->tokens[*index].end;
    size_t str_len = ti->tokens[*index].end - ti->tokens[*index].start;
    if (!strncmp("false", str_val, str_len)) {
        val = plist_new_bool(0);
    } else if (!strncmp("true", str_val, str_len)) {
        val = plist_new_bool(1);
    } else 
    // Check if the primitive value is "null" and create a corresponding plist node for it.
    // If the value represents a number, parse it as an integer or floating point number.
    // Handle potential representations of negative numbers and scientific notation.
    // If the value is an integer, create a plist node for it.
    // If the value is a floating-point number, parse the fractional and exponential parts
    // and create a plist node for the real number.
    // Log an error if any parsing errors occur or if the value cannot be represented.
    // <MASK>
    (*index)++;
    return val;
}