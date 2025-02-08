/* Concat MCC and MNC in temp buffer */
        temp_strbuf = wmem_strbuf_sized_new(actx->pinfo->pool,7,7);
        wmem_strbuf_append_printf(temp_strbuf,"%s",wmem_strbuf_get_str(last_mcc_strbuf));
        wmem_strbuf_append_printf(temp_strbuf,"%s",wmem_strbuf_get_str(mcc_mnc_strbuf));
        /* Update length of recovered MCC-MNC pair */
        string_len = (guint32)wmem_strbuf_get_len(temp_strbuf);
        mcc_mnc_string = wmem_strbuf_finalize(temp_strbuf);