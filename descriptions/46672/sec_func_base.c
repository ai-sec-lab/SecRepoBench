bool gsicc_setup_black_textvec(gs_gstate *pgs, gx_device *dev, bool is_text)
{
    gs_color_space *pcs_curr = gs_currentcolorspace_inline(pgs);
    gs_color_space *pcs_alt = gs_swappedcolorspace_inline(pgs);

    /* If neither space is ICC then we are not doing anything */
    if (!gs_color_space_is_ICC(pcs_curr) && !gs_color_space_is_ICC(pcs_alt))
        return false;

    /* Create a new object to hold the cs details */
    pgs->black_textvec_state = gsicc_blacktextvec_state_new(pgs->memory, is_text);
    if (pgs->black_textvec_state == NULL)
        return false; /* No error just move on */

    /* If curr space is ICC then store it */
    if  (gs_color_space_is_ICC(pcs_curr)) {
        rc_increment_cs(pcs_curr);  /* We are storing the cs. Will decrement when structure is released */
        pgs->black_textvec_state->pcs = pcs_curr;
        pgs->black_textvec_state->pcc = pgs->color[0].ccolor;
        cs_adjust_color_count(pgs, 1); /* The set_gray will do a decrement, only need if pattern */
        pgs->black_textvec_state->value[0] = pgs->color[0].ccolor->paint.values[0];

        if (is_white(pgs, pcs_curr, pgs->color[0].ccolor))
            gs_setgray(pgs, 1.0);
        else
            gs_setgray(pgs, 0.0);
    }

    /* If alt space is ICC then store it */
    if (gs_color_space_is_ICC(pcs_alt)) {
        rc_increment_cs(pcs_alt);  /* We are storing the cs. Will decrement when structure is released */
        pgs->black_textvec_state->pcs_alt = pcs_alt;

        gs_swapcolors_quick(pgs);  /* Have to swap for set_gray and adjust color count */
        pgs->black_textvec_state->pcc_alt = pgs->color[0].ccolor;
        cs_adjust_color_count(pgs, 1); /* The set_gray will do a decrement, only need if pattern */
        pgs->black_textvec_state->value[1] = pgs->color[0].ccolor->paint.values[0];

        if (is_white(pgs, pcs_alt, pgs->color[0].ccolor))
            gs_setgray(pgs, 1.0);
        else
            gs_setgray(pgs, 0.0);
        gs_swapcolors_quick(pgs);
    }

    pgs->black_textvec_state->is_fill = pgs->is_fill_color;
    return true; /* Need to clean up */
}