/* Find how many bytes we need for the cidbase value */
            /* We always store at least two bytes for the cidbase value */
            for (valuelen = 16; valuelen < 32 && (cidbase >> valuelen) > 0; valuelen += 1)
                DO_NOTHING;

            valuelen = ((valuelen + 7) & ~7) >> 3;

            /* The prefix is already directly in the gx_cmap_lookup_range_t
             * We need to store the lower and upper character codes, after lopping the prefix
             * off them. The upper and lower codes must be the same number of bytes.
             */
            j = sizeof(pdfi_cmap_range_map_t) + 2 * (stobj[i].size - preflen) + valuelen;

            pdfir = (pdfi_cmap_range_map_t *)gs_alloc_bytes(mem, j, "cmap_endcidrange_func(pdfi_cmap_range_map_t)");
            if (pdfir != NULL) {
                gx_cmap_lookup_range_t *gxr = &pdfir->range;
                pdfir->next = NULL;
                gxr->num_entries = 1;
                gxr->keys.data = (byte *)&(pdfir[1]);
                gxr->values.data = gxr->keys.data + 2 * (stobj[i].size - preflen);

                gxr->cmap = NULL;
                gxr->font_index = 0;
                gxr->key_is_range = true;
                gxr->value_type = cmap_range == &(pdficmap->cmap_range) ? CODE_VALUE_CID : CODE_VALUE_NOTDEF;
                gxr->key_prefix_size = preflen;
                gxr->key_size = stobj[i].size - gxr->key_prefix_size;
                memcpy(gxr->key_prefix, stobj[i].val.string, gxr->key_prefix_size);

                memcpy(gxr->keys.data, stobj[i].val.string + gxr->key_prefix_size, stobj[i].size - gxr->key_prefix_size);
                memcpy(gxr->keys.data + (stobj[i].size - gxr->key_prefix_size), stobj[i + 1].val.string + gxr->key_prefix_size, stobj[i + 1].size - gxr->key_prefix_size);

                gxr->keys.size = (stobj[i].size - gxr->key_prefix_size) + (stobj[i + 1].size - gxr->key_prefix_size);
                for (j = 0; j < valuelen; j++) {
                    gxr->values.data[j] = (cidbase >> ((valuelen - 1 - j) * 8)) & 255;
                }
                gxr->value_size = valuelen; /* I'm not sure.... */
                gxr->values.size = valuelen;
                if (cmap_insert_map(cmap_range, pdfir) < 0) break;
            }
            else {
                break;
            }