size_t offset;

    posision_x = posision_y = 0;
    max_x = max_y = 0;
    attributed_pan = 2;
    attributedpadding = 1;
    attributed_ph = attributed_pv = 0;
    repeat_count = 1;
    color_index = 0;
    background_color_index = 0;

    imsx = 2048;
    imsy = 2048;
    imbuf = (unsigned char *) AcquireQuantumMemory(imsx , imsy);

    if (imbuf == NULL) {
        return(MagickFalse);
    }

    for (n = 0; n < 16; n++) {
        sixel_palet[n] = sixel_default_color_table[n];
    }

    /* colors 16-231 are a 6x6x6 color cube */
    for (r = 0; r < 6; r++) {
        for (g = 0; g < 6; g++) {
            for (b = 0; b < 6; b++) {
                sixel_palet[n++] = SIXEL_RGB(r * 51, g * 51, b * 51);
            }
        }
    }
    /* colors 232-255 are a grayscale ramp, intentionally leaving out */
    for (i = 0; i < 24; i++) {
        sixel_palet[n++] = SIXEL_RGB(i * 11, i * 11, i * 11);
    }

    for (; n < SIXEL_PALETTE_MAX; n++) {
        sixel_palet[n] = SIXEL_RGB(255, 255, 255);
    }

    (void) ResetMagickMemory(imbuf, background_color_index, (size_t) imsx * imsy);

    while (*p != '\0') {
        if ((p[0] == '\033' && p[1] == 'P') || *p == 0x90) {
            if (*p == '\033') {
                p++;
            }

            p = get_params(++p, param, &n);

            if (*p == 'q') {
                p++;

                if (n > 0) {        /* Pn1 */
                    switch(param[0]) {
                    case 0:
                    case 1:
                        attributedpadding = 2;
                        break;
                    case 2:
                        attributedpadding = 5;
                        break;
                    case 3:
                        attributedpadding = 4;
                        break;
                    case 4:
                        attributedpadding = 4;
                        break;
                    case 5:
                        attributedpadding = 3;
                        break;
                    case 6:
                        attributedpadding = 3;
                        break;
                    case 7:
                        attributedpadding = 2;
                        break;
                    case 8:
                        attributedpadding = 2;
                        break;
                    case 9:
                        attributedpadding = 1;
                        break;
                    }
                }

                if (n > 2) {        /* Pn3 */
                    if (param[2] == 0) {
                        param[2] = 10;
                    }
                    attributed_pan = attributed_pan * param[2] / 10;
                    attributedpadding = attributedpadding * param[2] / 10;
                    if (attributed_pan <= 0) attributed_pan = 1;
                    if (attributedpadding <= 0) attributedpadding = 1;
                }
            }

        } else if ((p[0] == '\033' && p[1] == '\\') || *p == 0x9C) {
            break;
        } else if (*p == '"') {
            /* DECGRA Set Raster Attributes " Pan; Pad; Ph; Pv */
            p = get_params(++p, param, &n);

            if (n > 0) attributedpadding = param[0];
            if (n > 1) attributed_pan = param[1];
            if (n > 2 && param[2] > 0) attributed_ph = param[2];
            if (n > 3 && param[3] > 0) attributed_pv = param[3];

            if (attributed_pan <= 0) attributed_pan = 1;
            if (attributedpadding <= 0) attributedpadding = 1;

            if (imsx < attributed_ph || imsy < attributed_pv) {
                dmsx = imsx > attributed_ph ? imsx : attributed_ph;
                dmsy = imsy > attributed_pv ? imsy : attributed_pv;
                dmbuf = (unsigned char *) AcquireQuantumMemory(dmsx , dmsy);
                if (dmbuf == (unsigned char *) NULL) {
                    imbuf = (unsigned char *) RelinquishMagickMemory(imbuf);
                    return (MagickFalse);
                }
                (void) ResetMagickMemory(dmbuf, background_color_index, (size_t) dmsx * dmsy);
                for (y = 0; y < imsy; ++y) {
                    (void) CopyMagickMemory(dmbuf + dmsx * y, imbuf + imsx * y, imsx);
                }
                imbuf = (unsigned char *) RelinquishMagickMemory(imbuf);
                imsx = dmsx;
                imsy = dmsy;
                imbuf = dmbuf;
            }

        } else if (*p == '!') {
            /* DECGRI Graphics Repeat Introducer ! Pn Ch */
            p = get_params(++p, param, &n);

            if ((n > 0) && (param[0] > 0)) {
                repeat_count = param[0];
            }

        } else if (*p == '#') {
            /* DECGCI Graphics Color Introducer # Pc; Pu; Px; Py; Pz */
            p = get_params(++p, param, &n);

            if (n > 0) {
                if ((color_index = param[0]) < 0) {
                    color_index = 0;
                } else if (color_index >= SIXEL_PALETTE_MAX) {
                    color_index = SIXEL_PALETTE_MAX - 1;
                }
            }

            if (n > 4) {
                if (param[1] == 1) {            /* HLS */
                    if (param[2] > 360) param[2] = 360;
                    if (param[3] > 100) param[3] = 100;
                    if (param[4] > 100) param[4] = 100;
                    sixel_palet[color_index] = hls_to_rgb(param[2] * 100 / 360, param[3], param[4]);
                } else if (param[1] == 2) {    /* RGB */
                    if (param[2] > 100) param[2] = 100;
                    if (param[3] > 100) param[3] = 100;
                    if (param[4] > 100) param[4] = 100;
                    sixel_palet[color_index] = SIXEL_XRGB(param[2], param[3], param[4]);
                }
            }

        } else if (*p == '$') {
            /* DECGCR Graphics Carriage Return */
            p++;
            posision_x = 0;
            repeat_count = 1;

        } else if (*p == '-') {
            /* DECGNL Graphics Next Line */
            p++;
            posision_x  = 0;
            posision_y += 6;
            repeat_count = 1;

        } else if (*p >= '?' && *p <= '\177') {
            if (imsx < (posision_x + repeat_count) || imsy < (posision_y + 6)) {
                int nx = imsx * 2;
                int ny = imsy * 2;

                while (nx < (posision_x + repeat_count) || ny < (posision_y + 6)) {
                    nx *= 2;
                    ny *= 2;
                }

                dmsx = nx;
                dmsy = ny;
                dmbuf = (unsigned char *) AcquireQuantumMemory(dmsx , dmsy);
                if (dmbuf == (unsigned char *) NULL) {
                    imbuf = (unsigned char *) RelinquishMagickMemory(imbuf);
                    return (MagickFalse);
                }
                (void) ResetMagickMemory(dmbuf, background_color_index, (size_t) dmsx * dmsy);
                for (y = 0; y < imsy; ++y) {
                    (void) CopyMagickMemory(dmbuf + dmsx * y, imbuf + imsx * y, imsx);
                }
                imbuf = (unsigned char *) RelinquishMagickMemory(imbuf);
                imsx = dmsx;
                imsy = dmsy;
                imbuf = dmbuf;
            }

            if (color_index > max_color_index) {
                max_color_index = color_index;
            }
            if ((b = *(p++) - '?') == 0) {
                posision_x += repeat_count;

            } else {
                sixel_vertical_mask = 0x01;

                if (repeat_count <= 1) {
                    for (i = 0; i < 6; i++) {
                        if ((b & sixel_vertical_mask) != 0) {
                            offset=(size_t) imsx * (posision_y + i) + posision_x;
                            if (offset >= (size_t) imsx * imsy)
                              {
                                imbuf = (unsigned char *) RelinquishMagickMemory(imbuf);
                                return (MagickFalse);
                              }
                            imbuf[offset] = color_index;
                            if (max_x < posision_x) {
                                max_x = posision_x;
                            }
                            if (max_y < (posision_y + i)) {
                                max_y = posision_y + i;
                            }
                        }
                        sixel_vertical_mask <<= 1;
                    }
                    posision_x += 1;

                } else { /* repeat_count > 1 */
                    for (i = 0; i < 6; i++) {
                        if ((b & sixel_vertical_mask) != 0) {
                            c = sixel_vertical_mask << 1;
                            for (n = 1; (i + n) < 6; n++) {
                                if ((b & c) == 0) {
                                    break;
                                }
                                c <<= 1;
                            }
                            for (y = posision_y + i; y < posision_y + i + n; ++y) {
                                offset=(size_t) imsx * y + posision_x;
                                if (offset + repeat_count >= (size_t) imsx * imsy)
                                  {
                                    imbuf = (unsigned char *) RelinquishMagickMemory(imbuf);
                                    return (MagickFalse);
                                  }
                                (void) ResetMagickMemory(imbuf + offset, color_index, repeat_count);
                            }
                            if (max_x < (posision_x + repeat_count - 1)) {
                                max_x = posision_x + repeat_count - 1;
                            }
                            if (max_y < (posision_y + i + n - 1)) {
                                max_y = posision_y + i + n - 1;
                            }

                            i += (n - 1);
                            sixel_vertical_mask <<= (n - 1);
                        }
                        sixel_vertical_mask <<= 1;
                    }
                    posision_x += repeat_count;
                }
            }
            repeat_count = 1;
        } else {
            p++;
        }
    }