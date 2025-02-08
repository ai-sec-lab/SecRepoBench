memcpy(ptpriv->font_name.chars, fnamestr->data, fnamestr->length);
                        memcpy(ptpriv->key_name.chars, fnamestr->data, fnamestr->length);
                        ptpriv->font_name.size = ptpriv->key_name.size = fnamestr->length;