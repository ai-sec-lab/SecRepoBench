{
        plist_data_t data = plist_new_plist_data();
        data->type = PLIST_NULL;
        val = plist_new_node(data);
    } else if (isdigit(str_val[0]) || (str_val[0] == '-' && str_val+1 < str_end && isdigit(str_val[1]))) {
        char* endp = (char*)str_val;
        int64_t intpart = parse_decimal(str_val, str_end, &endp);
        if (endp >= str_end) {
            /* integer */
            val = plist_new_uint((uint64_t)intpart);
        } else if ((*endp == '.' && endp+1 < str_end && isdigit(*(endp+1))) || ((*endp == 'e' || *endp == 'E') && endp < str_end && (isdigit(*(endp+1)) || ((*(endp+1) == '-') && endp+1 < str_end && isdigit(*(endp+2)))))) {
            /* floating point */
            double dval = (double)intpart;
            char* fendp = endp;
            int err = 0;
            do {
                if (*endp == '.') {
                    fendp++;
                    int is_neg = (str_val[0] == '-');
                    double frac = 0;
                    double p = 0.1;
                    while (fendp < str_end && isdigit(*fendp)) {
                        frac = frac + (*fendp - '0') * p;
                        p *= 0.1;
                        fendp++;
                    }
                    if (is_neg) {
                        dval -= frac;
                    } else {
                        dval += frac;
                    }
                }
                if (fendp >= str_end) {
                    break;
                }
                if (fendp+1 < str_end && (*fendp == 'e' || *fendp == 'E') && (isdigit(*(fendp+1)) || ((*(fendp+1) == '-') && fendp+2 < str_end && isdigit(*(fendp+2))))) {
                    double exp = (double)parse_decimal(fendp+1, str_end, &fendp);
                    dval = dval * pow(10, exp);
                } else {
                    PLIST_JSON_ERR("%s: invalid character at offset %d when parsing floating point value\n", __func__, (int)(fendp - js));
                    err++;
                }
            } while (0);
            if (!err) {
                if (isinf(dval) || isnan(dval)) {
                   PLIST_JSON_ERR("%s: unrepresentable floating point value at offset %d when parsing numerical value\n", __func__, (int)(str_val - js));
                } else {
                    val = plist_new_real(dval);
                }
            }
        } else {
            PLIST_JSON_ERR("%s: invalid character at offset %d when parsing numerical value\n", __func__, (int)(endp - js));
        }
    } else {
        PLIST_JSON_ERR("%s: invalid primitive value '%.*s' encountered\n", __func__, (int)str_len, str_val);
    }