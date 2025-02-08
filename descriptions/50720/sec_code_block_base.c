int nlen = fnamestr->length > gs_font_name_max ? gs_font_name_max : fnamestr->length;
                        memcpy(ptpriv->font_name.chars, fnamestr->data, nlen);
                        memcpy(ptpriv->key_name.chars, fnamestr->data, nlen);
                        ptpriv->font_name.size = ptpriv->key_name.size = nlen;