int
gs_type2_interpret(gs_type1_state * pcis, const gs_glyph_data_t *pgd,
                   int *ignore_pindex)
{
    gs_font_type1 *pfont = pcis->pfont;
    gs_type1_data *pdata = &pfont->data;
    t1_hinter *h = &pcis->h;
    bool encrypted = pdata->lenIV >= 0;
    fixed cstack[ostack_size];
    cs_ptr csp;
#define clear CLEAR_CSTACK(cstack, csp)
    ip_state_t *ipsp = &pcis->ipstack[pcis->ips_count - 1];
    register const byte *cip;
    register crypt_state state;
    register int c;
    cs_ptr ap;
    bool vertical;
    int code = 0;

/****** FAKE THE REGISTRY ******/
    struct {
        float *values;
        uint size;
    } Registry[1];

    Registry[0].values = pcis->pfont->data.WeightVector.values;

    switch (pcis->init_done) {
        case -1:
            t1_hinter__init(h, pcis->path);
            break;
        case 0:
            gs_type1_finish_init(pcis);	/* sets origin */
            code = t1_hinter__set_mapping(h, &pcis->pgs->ctm,
                            &pfont->FontMatrix, &pfont->base->FontMatrix,
                            pcis->scale.x.log2_unit, pcis->scale.x.log2_unit,
                            pcis->scale.x.log2_unit - pcis->log2_subpixels.x,
                            pcis->scale.y.log2_unit - pcis->log2_subpixels.y,
                            pcis->origin.x, pcis->origin.y,
                            gs_currentaligntopixels(pfont->dir));
            if (code < 0)
                return code;
            code = t1_hinter__set_font_data(pfont->memory, h, 2, pdata, pcis->no_grid_fitting,
                            pcis->pfont->is_resource);
            if (code < 0)
                return code;
            break;
        default /*case 1 */ :
            break;
    }
    INIT_CSTACK(cstack, csp, pcis);

    if (pgd == 0)
        goto cont;
    ipsp->cs_data = *pgd;
    cip = pgd->bits.data;
    if (cip == 0)
        return (gs_note_error(gs_error_invalidfont));
    goto call;
    for (;;) {
        uint c0 = *cip++;

        charstring_next(c0, state, c, encrypted);
        if (c >= c_num1) {
            /* This is a number, decode it and push it on the stack. */

            if (c < c_pos2_0) {	/* 1-byte number */
                decode_push_num1(csp, cstack, c);
            } else if (c < cx_num4) {	/* 2-byte number */
                decode_push_num2(csp, cstack, c, cip, state, encrypted);
            } else if (c == cx_num4) {	/* 4-byte number */
                long lw;

                decode_num4(lw, cip, state, encrypted);
                /* 32-bit numbers are 16:16. */
                CS_CHECK_PUSH(csp, cstack);
                *++csp = arith_rshift(lw, 16 - _fixed_shift);
            } else		/* not possible */
                return_error(gs_error_invalidfont);
            goto pushed;
        }
#ifdef DEBUG
        if (gs_debug['1']) {
            static const char *const c2names[] =
            {char2_command_names};

            if (c2names[c] == 0)
                dmlprintf2(pfont->memory, "[1]"PRI_INTPTR": %02x??\n",
                           (intptr_t)(cip - 1), c);
            else
                dmlprintf3(pfont->memory, "[1]"PRI_INTPTR": %02x %s\n",
                           (intptr_t)(cip - 1), c, c2names[c]);
        }
#endif
        switch ((char_command) c) {

                /* Commands with identical functions in Type 1 and Type 2, */
                /* except for 'escape'. */

            case c_undef0:
            case c_undef2:
            case c_undef17:
                return_error(gs_error_invalidfont);
            // <MASK>
            case cx_escape:
                charstring_next(*cip, state, c, encrypted);
                ++cip;
#ifdef DEBUG
                if (gs_debug['1'] && c < char2_extended_command_count) {
                    static const char *const ce2names[] =
                    {char2_extended_command_names};

                    if (ce2names[c] == 0)
                        dmlprintf2(pfont->memory, "[1]"PRI_INTPTR": %02x??\n",
                                   (intptr_t)(cip - 1), c);
                    else
                        dmlprintf3(pfont->memory, "[1]"PRI_INTPTR": %02x %s\n",
                                   (intptr_t)(cip - 1), c, ce2names[c]);
                }
#endif
                switch ((char2_extended_command) c) {
                    case ce2_and:
                        if (!CS_CHECK_CSTACK_BOUNDS(&csp[-1], cstack))
                            return_error(gs_error_invalidfont);
                        csp[-1] = ((csp[-1] != 0) & (*csp != 0) ? fixed_1 : 0);
                        --csp;
                        break;
                    case ce2_or:
                        if (!CS_CHECK_CSTACK_BOUNDS(&csp[-1], cstack))
                            return_error(gs_error_invalidfont);
                        csp[-1] = (csp[-1] | *csp ? fixed_1 : 0);
                        --csp;
                        break;
                    case ce2_not:
                        *csp = (*csp ? 0 : fixed_1);
                        break;
                    case ce2_store:
                        if (!CS_CHECK_CSTACK_BOUNDS(&csp[-3], cstack))
                            return_error(gs_error_invalidfont);
                        {
                            int i, n = fixed2int_var(*csp);
                            int ind = fixed2int_var(csp[-3]);
                            int offs = fixed2int_var(csp[-2]);
                            float *to;
                            const fixed *from = pcis->transient_array + fixed2int_var(csp[-1]);

                            if (!CS_CHECK_TRANSIENT_BOUNDS(from, pcis->transient_array))
                                return_error(gs_error_invalidfont);

                            if (ind < countof(Registry)) {
                                to = Registry[ind].values + offs;
                                for (i = 0; i < n; ++i)
                                    to[i] = fixed2float(from[i]);
                            }
                        }
                        csp -= 4;
                        break;
                    case ce2_abs:
                        if (*csp < 0)
                            *csp = -*csp;
                        break;
                    case ce2_add:
                        if (!CS_CHECK_CSTACK_BOUNDS(&csp[-1], cstack))
                            return_error(gs_error_invalidfont);
                        csp[-1] += *csp;
                        --csp;
                        break;
                    case ce2_sub:
                        if (!CS_CHECK_CSTACK_BOUNDS(&csp[-1], cstack))
                            return_error(gs_error_invalidfont);
                        csp[-1] -= *csp;
                        --csp;
                        break;
                    case ce2_div:
                        if (!CS_CHECK_CSTACK_BOUNDS(&csp[-1], cstack))
                            return_error(gs_error_invalidfont);
                        if (*csp == 0)
                            return_error(gs_error_invalidfont);
                        csp[-1] = float2fixed((double)csp[-1] / *csp);
                        --csp;
                        break;
                    case ce2_load:
                        if (!CS_CHECK_CSTACK_BOUNDS(&csp[-2], cstack))
                            return_error(gs_error_invalidfont);
                        /* The specification says there is no j (starting index */
                        /* in registry array) argument.... */
                        {
                            int i, n = fixed2int_var(*csp);
                            int ind = fixed2int_var(csp[-2]);
                            const float *from;
                            fixed *to = pcis->transient_array + fixed2int_var(csp[-1]);

                            if (!CS_CHECK_TRANSIENT_BOUNDS(to, pcis->transient_array))
                                return_error(gs_error_invalidfont);
                            if (ind < countof(Registry)) {
                                from = Registry[ind].values;
                                for (i = 0; i < n; ++i)
                                    to[i] = float2fixed(from[i]);
                            }
                        }
                        csp -= 3;
                        break;
                    case ce2_neg:
                        *csp = -*csp;
                        break;
                    case ce2_eq:
                        if (!CS_CHECK_CSTACK_BOUNDS(&csp[-1], cstack))
                            return_error(gs_error_invalidfont);
                        csp[-1] = (csp[-1] == *csp ? fixed_1 : 0);
                        --csp;
                        break;
                    case ce2_drop:
                        if (!CS_CHECK_CSTACK_BOUNDS(&csp[-1], cstack))
                            return_error(gs_error_invalidfont);
                        --csp;
                        break;
                    case ce2_put:
                        if (!CS_CHECK_CSTACK_BOUNDS(&csp[-1], cstack))
                            return_error(gs_error_invalidfont);
                        {
                            fixed *to = pcis->transient_array + fixed2int_var(*csp);

                            if (!CS_CHECK_TRANSIENT_BOUNDS(to, pcis->transient_array))
                                return_error(gs_error_invalidfont);

                            *to = csp[-1];
                            csp -= 2;
                        }
                        break;
                    case ce2_get:
                        if (!CS_CHECK_CSTACK_BOUNDS(csp, cstack))
                            return_error(gs_error_invalidfont);
                        {
                            fixed *from = pcis->transient_array + fixed2int_var(*csp);
                            if (!CS_CHECK_TRANSIENT_BOUNDS(from, pcis->transient_array))
                                return_error(gs_error_invalidfont);

                            *csp = *from;
                        }
                        break;
                    case ce2_ifelse:
                        if (!CS_CHECK_CSTACK_BOUNDS(&csp[-3], cstack))
                            return_error(gs_error_invalidfont);
                        if (csp[-1] > *csp)
                            csp[-3] = csp[-2];
                        csp -= 3;
                        break;
                    case ce2_random:
                        CS_CHECK_PUSH(csp, cstack);
                        ++csp;
                        /****** NYI ******/
                        break;
                    case ce2_mul:
                        if (!CS_CHECK_CSTACK_BOUNDS(&csp[-1], cstack))
                            return_error(gs_error_invalidfont);
                        {
                            double prod = fixed2float(csp[-1]) * *csp;

                            csp[-1] =
                                (prod > max_fixed ? max_fixed :
                                 prod < min_fixed ? min_fixed : (fixed)prod);
                        }
                        --csp;
                        break;
                    case ce2_sqrt:
                        if (*csp >= 0)
                            *csp = float2fixed(sqrt(fixed2float(*csp)));
                        break;
                    case ce2_dup:
                        CS_CHECK_PUSH(csp, cstack);
                        csp[1] = *csp;
                        ++csp;
                        break;
                    case ce2_exch:
                        if (!CS_CHECK_CSTACK_BOUNDS(&csp[-1], cstack))
                            return_error(gs_error_invalidfont);
                        {
                            fixed top = *csp;

                            *csp = csp[-1], csp[-1] = top;
                        }
                        break;
                    case ce2_index:
                        if (!CS_CHECK_CSTACK_BOUNDS(&csp[-1], cstack))
                            return_error(gs_error_invalidfont);
                        *csp =
                            (*csp < 0 ? csp[-1] : csp[-1 - fixed2int_var(csp[-1])]);
                        break;
                    case ce2_roll:
                        if (!CS_CHECK_CSTACK_BOUNDS(&csp[-1], cstack))
                            return_error(gs_error_invalidfont);
                        {
                            int distance = fixed2int_var(*csp);
                            int count = fixed2int_var(csp[-1]);
                            cs_ptr bot;

                            csp -= 2;
                            if (count < 0 || count > csp + 1 - cstack)
                                return_error(gs_error_invalidfont);
                            if (count == 0)
                                break;
                            if (distance < 0)
                                distance = count - (-distance % count);
                            bot = csp + 1 - count;
                            while (--distance >= 0) {
                                fixed top = *csp;

                                memmove(bot + 1, bot,
                                        (count - 1) * sizeof(fixed));
                                *bot = top;
                            }
                        }
                        break;
                    case ce2_hflex:
                        if (!CS_CHECK_CSTACK_BOUNDS(&csp[-5], cstack))
                            return_error(gs_error_invalidfont);
                        CS_CHECK_PUSHN(csp, cstack, 6);
                        csp[6] = fixed_half;	/* fd/100 */
                        csp[4] = *csp, csp[5] = 0;	/* dx6, dy6 */
                        csp[2] = csp[-1], csp[3] = -csp[-4];	/* dx5, dy5 */
                        *csp = csp[-2], csp[1] = 0;	/* dx4, dy4 */
                        csp[-2] = csp[-3], csp[-1] = 0;		/* dx3, dy3 */
                        csp[-3] = csp[-4], csp[-4] = csp[-5];	/* dx2, dy2 */
                        csp[-5] = 0;	/* dy1 */
                        csp += 6;
                        goto flex;
                    case ce2_flex:
                        if (!CS_CHECK_CSTACK_BOUNDS(&csp[-12], cstack))
                            return_error(gs_error_invalidfont);
                        *csp /= 100;	/* fd/100 */
                        goto flex;
                    case ce2_hflex1:
                        if (!CS_CHECK_CSTACK_BOUNDS(&csp[-3], cstack))
                            return_error(gs_error_invalidfont);
                        CS_CHECK_PUSHN(csp, cstack, 4);
                        csp[4] = fixed_half;	/* fd/100 */
                        csp[2] = *csp;          /* dx6 */
                        csp[3] = -(csp[-7] + csp[-5] + csp[-1]);	/* dy6 */
                        *csp = csp[-2], csp[1] = csp[-1];	/* dx5, dy5 */
                        csp[-2] = csp[-3], csp[-1] = 0;		/* dx4, dy4 */
                        csp[-3] = 0;	/* dy3 */
                        csp += 4;
                        goto flex;
                    case ce2_flex1:
                        if (!CS_CHECK_CSTACK_BOUNDS(&csp[-10], cstack))
                            return_error(gs_error_invalidfont);
                        CS_CHECK_PUSHN(csp, cstack, 2);
                        {
                            fixed dx = csp[-10] + csp[-8] + csp[-6] + csp[-4] + csp[-2];
                            fixed dy = csp[-9] + csp[-7] + csp[-5] + csp[-3] + csp[-1];

                            if (any_abs(dx) > any_abs(dy))
                                csp[1] = -dy;	/* d6 is dx6 */
                            else
                                csp[1] = *csp, *csp = -dx;	/* d6 is dy6 */
                        }
                        csp[2] = fixed_half;	/* fd/100 */
                        csp += 2;
flex:			{
                            fixed x_join = csp[-12] + csp[-10] + csp[-8];
                            fixed y_join = csp[-11] + csp[-9] + csp[-7];
                            fixed x_end = x_join + csp[-6] + csp[-4] + csp[-2];
                            fixed y_end = y_join + csp[-5] + csp[-3] + csp[-1];
                            gs_point join, end;
                            double flex_depth;

                            if ((code =
                                 gs_distance_transform(fixed2float(x_join),
                                                       fixed2float(y_join),
                                                       &ctm_only(pcis->pgs),
                                                       &join)) < 0 ||
                                (code =
                                 gs_distance_transform(fixed2float(x_end),
                                                       fixed2float(y_end),
                                                       &ctm_only(pcis->pgs),
                                                       &end)) < 0
                                )
                                return code;
                            /*
                             * Use the X or Y distance depending on whether
                             * the curve is more horizontal or more
                             * vertical.
                             */
                            if (any_abs(end.y) > any_abs(end.x))
                                flex_depth = join.x;
                            else
                                flex_depth = join.y;
                            if (fabs(flex_depth) < fixed2float(*csp)) {
                                /* Do flex as line. */
                                code = t1_hinter__rlineto(h, x_end, y_end);
                            } else {
                                /*
                                 * Do flex as curve.  We can't jump to rrc,
                                 * because the flex operators don't clear
                                 * the stack (!).
                                 */
                                code = t1_hinter__rcurveto(h,
                                        csp[-12], csp[-11], csp[-10],
                                        csp[-9], csp[-8], csp[-7]);
                                if (code < 0)
                                    return code;
                                code = t1_hinter__rcurveto(h,
                                        csp[-6], csp[-5], csp[-4],
                                        csp[-3], csp[-2], csp[-1]);
                            }
                            if (code < 0)
                                return code;
                        }
                        clear;
                        continue;
                }
                break;

                /* Fill up the dispatch up to 32. */

            case_c2_undefs:
            default:		/* pacify compiler */
                return_error(gs_error_invalidfont);
        }
    }
}