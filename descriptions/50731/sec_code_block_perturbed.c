if (code < 0) {
        if (code != gs_error_undefined) {
            goto errorExit;
        }
        info->BPC = 1;
    }
    else if (info->BPC != 1 && info->BPC != 2 && info->BPC != 4 && info->BPC != 8 && info->BPC != 16) {
        code = gs_note_error(gs_error_rangecheck);
        goto errorExit;
    }