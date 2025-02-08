base_type = gs_color_space_get_index(pcs_base);
    if (base_type == gs_color_space_index_Pattern || base_type == gs_color_space_index_Indexed) {
        code = gs_note_error(gs_error_typecheck);
        goto exit;
    }

    (void)pcs_base->type->install_cspace(pcs_base, ctx->pgs);