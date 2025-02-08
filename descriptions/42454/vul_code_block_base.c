memcpy(str, (const char *)name->data, name->length);
            str[name->length] = '\0';
            dmprintf1(ctx->memory, "NAME:%s\n", str);