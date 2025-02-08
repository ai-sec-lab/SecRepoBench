FT_Render_Mode mode = FT_RENDER_MODE_MONO;

                ft_error = FT_Render_Glyph(ft_face->glyph, mode);
                if (ft_error != 0) {
                    (*a_glyph) = NULL;
                    return (gs_error_VMerror);
                }