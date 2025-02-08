memcpy(str, (const char *)subtype->data, subtype->length);
        str[subtype->length] = '\0';
        dbgmprintf1(ctx->memory, "ANNOT: No handler for subtype %s\n", str);

        /* Not necessarily an error? We can just render the AP if there is one */
        render_done = false;