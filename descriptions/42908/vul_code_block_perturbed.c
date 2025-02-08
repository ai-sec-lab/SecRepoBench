memcpy(str, (const char *)type->data, type->length);
        str[type->length] = '\0';
        dmprintf1(ctx->memory, "WARNING: OC dict type is %s, expected OCG or OCMD\n", str);