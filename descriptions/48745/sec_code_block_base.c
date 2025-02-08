ft_error =
                FT_Open_Face(s->freetype_library, &open_args, a_font->subfont,
                             &ft_face);
            if (ft_error) {
                delete_inc_int (a_server, ft_inc_int);
                if (data_owned)
                    FF_free(s->ftmemory, own_font_data);
                return ft_to_gs_error(ft_error);
            }