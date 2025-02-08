int length = name->length;

            if (length > 0) {
                if (length > 100)
                    length = 99;

                memcpy(str, (const char *)name->data, length);
                str[length] = '\0';
                dmprintf1(ctx->memory, "NAME:%s\n", str);
            }