bool gsicc_setup_black_textvec(gs_gstate *pgs, gx_device *dev, bool istextmode)
{
    gs_color_space *pcs_curr = gs_currentcolorspace_inline(pgs);
    gs_color_space *pcs_alt = gs_swappedcolorspace_inline(pgs);

    /* If neither space is ICC then we are not doing anything */
    if (!gs_color_space_is_ICC(pcs_curr) && !gs_color_space_is_ICC(pcs_alt))
        return false;

    /* Create a new object to hold the cs details */
    pgs->black_textvec_state = gsicc_blacktextvec_state_new(pgs->memory, istextmode);
    if (pgs->black_textvec_state == NULL)
        return false; /* No error just move on */

    /* If curr space is ICC then store it */
    // <MASK>

    pgs->black_textvec_state->is_fill = pgs->is_fill_color;
    return true; /* Need to clean up */
}