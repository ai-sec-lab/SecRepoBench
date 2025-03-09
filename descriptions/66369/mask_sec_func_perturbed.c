static sam_hrec_rg_t *cram_encode_aux(cram_fd *filedescriptor, bam_seq_t *b,
                                      cram_container *c,
                                      cram_slice *s, cram_record *cr,
                                      int verbatim_NM, int verbatim_MD,
                                      int NM, kstring_t *MD, int cf_tag,
                                      int no_ref, int *err) {
    char *aux, *orig;
    sam_hrec_rg_t *brg = NULL;
    int aux_size = bam_get_l_aux(b);
    const char *aux_end = bam_data_end(b);
    cram_block *td_b = c->comp_hdr->TD_blk;
    int TD_blk_size = BLOCK_SIZE(td_b), new;
    char *key;
    khint_t k;

    if (err) *err = 1;

    orig = aux = (char *)bam_aux(b);


    // cF:i  => Extra CRAM bit flags.
    // 1:  Don't auto-decode MD (may be invalid)
    // 2:  Don't auto-decode NM (may be invalid)
    if (cf_tag && CRAM_MAJOR_VERS(filedescriptor->version) < 4) {
        // Temporary copy of aux so we can ammend it.
        aux = malloc(aux_size+4);
        if (!aux)
            return NULL;

        memcpy(aux, orig, aux_size);
        aux[aux_size++] = 'c';
        aux[aux_size++] = 'F';
        aux[aux_size++] = 'C';
        aux[aux_size++] = cf_tag;
        orig = aux;
        aux_end = aux + aux_size;
    }

    // Copy aux keys to td_b and aux values to slice aux blocks
    while (aux_end - aux >= 1 && aux[0] != 0) {
        int r;

        // Room for code + type + at least 1 byte of data
        if (aux - orig >= aux_size - 3)
            goto err;

        // RG:Z
        // <MASK>
        tm->blk->m = tm->m;
    }

    // FIXME: sort BLOCK_DATA(td_b) by char[3] triples

    // And and increment TD hash entry
    BLOCK_APPEND_CHAR(td_b, 0);

    // Duplicate key as BLOCK_DATA() can be realloced to a new pointer.
    key = string_ndup(c->comp_hdr->TD_keys,
                      (char *)BLOCK_DATA(td_b) + TD_blk_size,
                      BLOCK_SIZE(td_b) - TD_blk_size);
    if (!key)
        goto block_err;
    k = kh_put(m_s2i, c->comp_hdr->TD_hash, key, &new);
    if (new < 0) {
        goto err;
    } else if (new == 0) {
        BLOCK_SIZE(td_b) = TD_blk_size;
    } else {
        kh_val(c->comp_hdr->TD_hash, k) = c->comp_hdr->nTL;
        c->comp_hdr->nTL++;
    }

    cr->TL = kh_val(c->comp_hdr->TD_hash, k);
    if (cram_stats_add(c->stats[DS_TL], cr->TL) < 0)
        goto block_err;

    if (orig != (char *)bam_aux(b))
        free(orig);

    if (err) *err = 0;

    return brg;

 err:
 block_err:
    if (orig != (char *)bam_aux(b))
        free(orig);
    return NULL;
}