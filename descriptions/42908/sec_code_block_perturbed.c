memcpy(str, (const char *)type->data, type->length < 100 ? type->length : 99);
        str[type->length < 100 ? type->length : 99] = '\0';
        dmprintf1(ctx->memory, "WARNING: OC dict type is %s, expected OCG or OCMD\n", str);