case c2_rcurveline:
                for (ap = cstack; ap + 5 <= csp; ap += 6) {
                    code = t1_hinter__rcurveto(h, ap[0], ap[1], ap[2], ap[3],
                                            ap[4], ap[5]);
                    if (code < 0)
                        return code;
                }
                code = t1_hinter__rlineto(h, ap[0], ap[1]);
                goto cc;
            case c2_rlinecurve:
                for (ap = cstack; ap + 7 <= csp; ap += 2) {
                    code = t1_hinter__rlineto(h, ap[0], ap[1]);
                    if (code < 0)
                        return code;
                }
                code = t1_hinter__rcurveto(h, ap[0], ap[1], ap[2], ap[3],
                                        ap[4], ap[5]);
  move:
  cc:
                if (code < 0)
                    return code;
                goto pp;
            case c2_vvcurveto:
                ap = cstack;
                {
                    int n = csp + 1 - cstack;
                    fixed dxa = (n & 1 ? *ap++ : 0);

                    for (; ap + 3 <= csp; ap += 4) {
                        code = t1_hinter__rcurveto(h, dxa, ap[0], ap[1], ap[2],
                                                fixed_0, ap[3]);
                        if (code < 0)
                            return code;
                        dxa = 0;
                    }
                }
                goto pp;
            case c2_hhcurveto:
                ap = cstack;
                {
                    int n = csp + 1 - cstack;
                    fixed dya = (n & 1 ? *ap++ : 0);

                    for (; ap + 3 <= csp; ap += 4) {
                        code = t1_hinter__rcurveto(h, ap[0], dya, ap[1], ap[2],
                                                ap[3], fixed_0);
                        if (code < 0)
                            return code;
                        dya = 0;
                    }
                }
              pp:
                clear;
                continue;
            case c2_shortint:
                {
                    int c1, c2;

                    charstring_next(*cip, state, c1, encrypted);
                    ++cip;
                    charstring_next(*cip, state, c2, encrypted);
                    ++cip;
                    CS_CHECK_PUSH(csp, cstack);
                    *++csp = int2fixed((((c1 ^ 0x80) - 0x80) << 8) + c2);
                }
  pushed:       if_debug3m('1', pfont->memory, "[1]%d: (%d) %f\n",
                           (int)(csp - cstack), c, fixed2float(*csp));
                continue;
            case c2_callgsubr:
                if (CS_CHECK_CSTACK_BOUNDS(csp, cstack)) {
                    CS_CHECK_IPSTACK(&(ipsp[1]), pcis->ipstack);
                    c = fixed2int_var(*csp) + pdata->gsubrNumberBias;
                    code = pdata->procs.subr_data
                        (pfont, c, true, &ipsp[1].cs_data);
  subr:
                    if (code < 0) {
                        /* Calling a Subr with an out-of-range index is clearly a error:
                         * the Adobe documentation says the results of doing this are
                         * undefined. However, we have seen a PDF file produced by Adobe
                         * PDF Library 4.16 that included a Type 2 font that called an
                         * out-of-range Subr, and Acrobat Reader did not signal an error.
                         * Therefore, we ignore such calls.
                         */
                        cip++;
                        continue;
                    }
                    --csp;
                    ipsp->ip = cip, ipsp->dstate = state;
                    ipsp->ip_end = endp;
                    ++ipsp;
                    cip = ipsp->cs_data.bits.data;
                    endp = cip + ipsp->cs_data.bits.size;
  call:
                    state = crypt_charstring_seed;
                    if (encrypted) {
                        int skip = pdata->lenIV;

                        /* Skip initial random bytes */
                        for (; skip > 0; ++cip, --skip)
                            decrypt_skip_next(*cip, state);
                    }
                    continue;
                } else {
                    cip++;
                    continue;
                }
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
                        if (!CS_CHECK_CSTACK_BOUNDS(csp, cstack))
                            return_error(gs_error_invalidfont);
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
                        if (!CS_CHECK_CSTACK_BOUNDS(csp, cstack))
                            return_error(gs_error_invalidfont);
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
                        if (!CS_CHECK_CSTACK_BOUNDS(csp, cstack))
                            return_error(gs_error_invalidfont);
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
                        if (!CS_CHECK_CSTACK_BOUNDS(csp, cstack))
                            return_error(gs_error_invalidfont);
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