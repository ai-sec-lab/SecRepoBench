if (b0 == 15) {
                offsets->charset_off = args[0].ival;
            }

            if (b0 == 16) {
                offsets->encoding_off = args[0].ival;
            }

            /* some CFF file offsets */

            if (b0 == 17) {
                font->charstrings = font->cffdata + args[0].ival;
            }

            if (b0 == 18) {
                offsets->private_size = args[0].ival;
                offsets->private_off = args[1].ival;
                /* Catch a broken font with a self referencing Private dict */
                if (topdict == true)
                    do_priv = offsets->private_size > 0 ? true : false;
                else {
                    do_priv = false;
                    code = gs_error_invalidfont;
                    break;
                }
            }

            if (b0 == 19) {
                font->subrs = font->cffdata + offset + args[0].ival;
            }