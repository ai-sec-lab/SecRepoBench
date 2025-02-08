if (code < 0) {
        if (code != gs_error_undefined) {
            goto errorExit;
        }
        info->BPC = 1;
    }