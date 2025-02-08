/* We need to null-terminate key for flb_warn, as it expects
                 * a null-terminated string, which key is not guaranteed
                 * to be */
                char *nt_key = flb_malloc(key_len + 1);
                if (nt_key != NULL) {
                    memcpy(nt_key, key, key_len);
                    nt_key[key_len] = '\0';
                    flb_warn("[PARSER] key=%s cast error. save as string.", nt_key);
                    flb_free(nt_key);
                }