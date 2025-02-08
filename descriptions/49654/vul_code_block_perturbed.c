if (cidname == NULL || pdfi_type_of(cidname) != PDF_NAME
         || fsprefixlen + cidname->length >= gp_file_name_sizeof)
            goto exit;