int code = 0;
    int num_channels = xc->num_channels;
    int bpc = pdev->devn_params.bitspercomponent;
    int chan_idx;
    int chan_names_len = 0;
    int sep_num;
    const devn_separation_name *separation_name;

    psd_write(xc, (const byte *)"8BPS", 4); /* Signature */
    psd_write_16(xc, 1); /* Version - Always equal to 1*/
    /* Reserved 6 Bytes - Must be zero */
    psd_write_32(xc, 0);
    psd_write_16(xc, 0);
    psd_write_16(xc, (bits16)num_channels); /* Channels (2 Bytes) - Supported range is 1 to 56 */
    psd_write_32(xc, xc->height); /* Rows */
    psd_write_32(xc, xc->width); /* Columns */
    psd_write_16(xc, bpc); /* Depth - 1, 8 and 16 */
    /* Modes: Bitmap=0, Grayscale=1, RGB=3, CMYK=4 MultiChannel=7 Lab=9 */
    psd_write_16(xc, (bits16) xc->base_num_channels);  /* We use 1, 3 or 4. */

    /* Color Mode Data */
    psd_write_32(xc, 0); 	/* No color mode data */

    /* Image Resources */

    /* Channel Names */
    for (chan_idx = NUM_CMYK_COMPONENTS; chan_idx < xc->num_channels; chan_idx++) {
        fixed_colorant_name n = pdev->devn_params.std_colorant_names[chan_idx];
        if (n == NULL)
            break;
        chan_names_len += strlen(n) + 1;
    }
    for (; chan_idx < xc->num_channels; chan_idx++) {
        sep_num = xc->chnl_to_orig_sep[chan_idx] - NUM_CMYK_COMPONENTS;
        separation_name = &(pdev->devn_params.separations.names[sep_num]);
        chan_names_len += (separation_name->size + 1);
    }
    psd_write_32(xc, 12 + (chan_names_len + (chan_names_len % 2))
                        + (12 + (14 * (xc->num_channels - xc->base_num_channels)))
                        + 28);
    psd_write(xc, (const byte *)"8BIM", 4);
    psd_write_16(xc, 1006); /* 0x03EE */
    psd_write_16(xc, 0); /* PString */
    psd_write_32(xc, chan_names_len + (chan_names_len % 2));
    for (chan_idx = NUM_CMYK_COMPONENTS; chan_idx < xc->num_channels; chan_idx++) {
        int len;
        fixed_colorant_name n = pdev->devn_params.std_colorant_names[chan_idx];
        if (n == NULL)
            break;
        len = strlen(n);
        psd_write_8(xc, (byte)len);
        psd_write(xc, (const byte *)n, len);
    }
    for (; chan_idx < xc->num_channels; chan_idx++) {
        sep_num = xc->chnl_to_orig_sep[chan_idx] - NUM_CMYK_COMPONENTS;
        separation_name = &(pdev->devn_params.separations.names[sep_num]);
        psd_write_8(xc, (byte) separation_name->size);
        psd_write(xc, separation_name->data, separation_name->size);
    }
    if (chan_names_len % 2)
        psd_write_8(xc, 0); /* pad */

    /* DisplayInfo - Colors for each spot channels */
    psd_write(xc, (const byte *)"8BIM", 4);
    psd_write_16(xc, 1007); /* 0x03EF */
    psd_write_16(xc, 0); /* PString */
    psd_write_32(xc, 14 * (xc->num_channels - xc->base_num_channels)); /* Length */
    for (chan_idx = NUM_CMYK_COMPONENTS; chan_idx < xc->num_channels; chan_idx++) {
        sep_num = xc->chnl_to_orig_sep[chan_idx] - NUM_CMYK_COMPONENTS;
        psd_write_16(xc, 02); /* CMYK */
        /* PhotoShop stores all component values as if they were additive. */
        if (pdev->equiv_cmyk_colors.color[sep_num].color_info_valid) {
#define convert_color(component) ((bits16)((65535 * ((double)\
    (frac_1 - pdev->equiv_cmyk_colors.color[sep_num].component)) / frac_1)))
            psd_write_16(xc, convert_color(c)); /* Cyan */
            psd_write_16(xc, convert_color(m)); /* Magenta */
            psd_write_16(xc, convert_color(y)); /* Yellow */
            psd_write_16(xc, convert_color(k)); /* Black */
#undef convert_color
        } else {
            /* This is a bit of a hack, introduced for the psdcmykog device
             * so that we get a reasonable approximation for the colors out
             * even when used without the appropriate profile. */
            fixed_colorant_name sepname = get_sep_name(pdev, chan_idx);
            if (sepname && !strcmp(sepname, "Artifex Orange")) {
                psd_write_16(xc, 0xfbde); /* Cyan */
                psd_write_16(xc, 0x7376); /* Magenta */
                psd_write_16(xc, 0x0000); /* Yellow */
                psd_write_16(xc, 0xffff); /* Black */
            } else if (sepname && !strcmp(sepname, "Artifex Green")) {
                psd_write_16(xc, 0x0000); /* Cyan */
                psd_write_16(xc, 0xe33d); /* Magenta */
                psd_write_16(xc, 0x0000); /* Yellow */
                psd_write_16(xc, 0xf8c8); /* Black */
            } else {
                /* Else set C = M = Y = 0, K = 1 */
                psd_write_16(xc, 65535); /* Cyan */
                psd_write_16(xc, 65535); /* Magenta */
                psd_write_16(xc, 65535); /* Yellow */
                psd_write_16(xc, 0); /* Black */
            }
        }
        psd_write_16(xc, 0); /* Opacity 0 to 100 */
        psd_write_8(xc, 2); /* Don't know */
        psd_write_8(xc, 0); /* Padding - Always Zero */
    }

    /* Image resolution */
    psd_write(xc, (const byte *)"8BIM", 4);
    psd_write_16(xc, 1005); /* 0x03ED */
    psd_write_16(xc, 0); /* PString */
    psd_write_32(xc, 16); /* Length */
                /* Resolution is specified as a fixed 16.16 bits */
    psd_write_32(xc, (int) (pdev->HWResolution[0] * 0x10000 * xc->width / pdev->width + 0.5));
    psd_write_16(xc, 1);	/* width:  1 --> resolution is pixels per inch */
    psd_write_16(xc, 1);	/* width:  1 --> resolution is pixels per inch */
    psd_write_32(xc, (int) (pdev->HWResolution[1] * 0x10000 * xc->height / pdev->height + 0.5));
    psd_write_16(xc, 1);	/* height:  1 --> resolution is pixels per inch */
    psd_write_16(xc, 1);	/* height:  1 --> resolution is pixels per inch */
