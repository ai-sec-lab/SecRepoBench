long long intpart = strtol(str_val, &endp, 10);
        if (endp >= str_end) {
            /* integer */
            val = plist_new_uint((uint64_t)intpart);
        } else if (*endp == '.' && endp+1 < str_end && isdigit(*(endp+1))) {
            /* float */
            char* fendp = endp+1;
            while (isdigit(*fendp) && fendp < str_end) fendp++;
            if ((fendp > endp+1 && fendp >= str_end) || (fendp+2 < str_end && (*fendp == 'e' || *fendp == 'E') && (*(fendp+1) == '+' || *(fendp+1) == '-') && isdigit(*(fendp+2)))) {
                double dval = atof(str_val);
                val = plist_new_real(dval);
            } else {
                PLIST_JSON_ERR("%s: invalid character at offset %d when parsing floating point value\n", __func__, (int)(fendp - js));
            }
        } else {
            PLIST_JSON_ERR("%s: invalid character at offset %d when parsing numerical value\n", __func__, (int)(endp - js));
        }