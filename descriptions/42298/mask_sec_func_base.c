static int
pdfi_read_cff_dict(byte *p, byte *e, pdfi_gs_cff_font_priv *ptpriv, cff_font_offsets *offsets)
{
    pdfi_cff_font_priv *font = &ptpriv->pdfcffpriv;
    // <MASK>

    /* recurse for the private dictionary */
    if (do_priv && code >= 0) {
        byte *dend = font->cffdata + offsets->private_off + offsets->private_size;

        if (dend > font->cffend)
            dend = font->cffend;

        if (p == NULL)
            code = gs_error_invalidfont;
        else
            code = pdfi_read_cff_dict(font->cffdata + offsets->private_off, dend, ptpriv, offsets);

        if (code < 0)
            dmprintf(ptpriv->memory, "CFF: cannot read private dictionary");
    }

    return code;
}