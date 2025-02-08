if (code < 0)
            goto finish;
        code = cos_dict_put_c_key_int((cos_dict_t *)writer.pres->object, "/Length1", stell(&poss));
        if (code < 0)
            goto finish;