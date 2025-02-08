case c_callsubr:
                if (CS_CHECK_CSTACK_BOUNDS(csp, cstack)) {
                    CS_CHECK_IPSTACK(&(ipsp[1]), pcis->ipstack);
                    c = fixed2int_var(*csp) + pdata->subroutineNumberBias;
                    code = pdata->procs.subr_data
                        (pfont, c, false, &ipsp[1].cs_data);
                    goto subr;
                }
                else {
                    /* Consider a missing index to be "out-of-range", and see above
                     * comment.
                     */
                    cip++;
                    continue;
                }
            case c_return:
                gs_glyph_data_free(&ipsp->cs_data, "gs_type2_interpret");
                --ipsp;
  cont:         if (ipsp < pcis->ipstack || ipsp->ip == 0)
                    return (gs_note_error(gs_error_invalidfont));
                cip = ipsp->ip;
                state = ipsp->dstate;
                continue;
            case c_undoc15:
                /* See gstype1.h for information on this opcode. */
                clear;
                continue;

                /* Commands with similar but not identical functions */
                /* in Type 1 and Type 2 charstrings. */

            case cx_hstem:
                goto hstem;
            case cx_vstem:
                goto vstem;
            case cx_vmoveto:
                if (CS_CHECK_CSTACK_BOUNDS(csp, cstack)) {
                    check_first_operator(csp > cstack);
                    code = t1_hinter__rmoveto(h, 0, *csp);
                    goto move;
                }
                else {
                    return_error(gs_error_invalidfont);
                }
            case cx_rlineto:
                for (ap = cstack; ap + 1 <= csp; ap += 2) {
                    code = t1_hinter__rlineto(h, ap[0], ap[1]);
                    if (code < 0)
                        return code;
                }
                goto pp;
            case cx_hlineto:
                vertical = false;
                goto hvl;
            case cx_vlineto:
                vertical = true;
              hvl:for (ap = cstack; ap <= csp; vertical = !vertical, ++ap) {
                    if (vertical) {
                        code = t1_hinter__rlineto(h, 0, ap[0]);
                    } else {
                        code = t1_hinter__rlineto(h, ap[0], 0);
                    }
                    if (code < 0)
                        return code;
                }
                goto pp;
            case cx_rrcurveto:
                for (ap = cstack; ap + 5 <= csp; ap += 6) {
                    code = t1_hinter__rcurveto(h, ap[0], ap[1], ap[2],
                                            ap[3], ap[4], ap[5]);
                    if (code < 0)
                        return code;
                }
                goto pp;
            case cx_endchar:
                /*
                 * It is a feature of Type 2 CharStrings that if endchar is
                 * invoked with 4 or 5 operands, it is equivalent to the
                 * Type 1 seac operator. In this case, the asb operand of
                 * seac is missing: we assume it is the same as the
                 * l.s.b. of the accented character.  This feature was
                 * undocumented until the 16 March 2000 version of the Type
                 * 2 Charstring Format specification, but, thankfully, is
                 * described in that revision.
                 */
                if (csp >= cstack + 3) {
                    check_first_operator(csp > cstack + 3);
                    code = gs_type1_seac(pcis, cstack, 0, ipsp);
                    if (code < 0)
                        return code;
                    clear;
                    cip = ipsp->cs_data.bits.data;
                    goto call;
                }
                /*
                 * This might be the only operator in the charstring.
                 * In this case, there might be a width on the stack.
                 */
                check_first_operator(csp >= cstack);
                if (pcis->seac_accent < 0) {
                    code = t1_hinter__endglyph(h);
                    if (code < 0)
                        return code;
                    code = gx_setcurrentpoint_from_path(pcis->pgs, pcis->path);
                    if (code < 0)
                        return code;
                } else {
                    t1_hinter__setcurrentpoint(h, pcis->save_adxy.x + pcis->origin_offset.x,
                                                  pcis->save_adxy.y + pcis->origin_offset.y);
                    code = t1_hinter__end_subglyph(h);
                    if (code < 0)
                        return code;
                }
                code = gs_type1_endchar(pcis);
                if (code == 1) {
                    /*
                     * Reset the total hint count so that hintmask will
                     * parse its following data correctly.
                     * (gs_type1_endchar already reset the actual hint
                     * tables.)
                     */
                    pcis->num_hints = 0;
                    /* do accent of seac */
                    ipsp = &pcis->ipstack[pcis->ips_count - 1];
                    cip = ipsp->cs_data.bits.data;
                    goto call;
                }
                return code;
            case cx_rmoveto:
                /* See vmoveto above re closing the subpath. */
                check_first_operator(!((csp - cstack) & 1));
                if (CS_CHECK_CSTACK_BOUNDS(&csp[-1], cstack)) {
                    if (csp > cstack + 1) {
                      /* Some Type 2 charstrings omit the vstemhm operator before rmoveto,
                         even though this is only allowed before hintmask and cntrmask.
                         Thanks to Felix Pahl.
                       */
                      type2_vstem(pcis, csp - 2, cstack);
                      cstack [0] = csp [-1];
                      cstack [1] = csp [ 0];
                      csp = cstack + 1;
                    }
                    code = t1_hinter__rmoveto(h, csp[-1], *csp);
                    goto move;
                }
                else
                    return_error(gs_error_invalidfont);
            case cx_hmoveto:
                if (CS_CHECK_CSTACK_BOUNDS(csp, cstack)) {
                    /* See vmoveto above re closing the subpath. */
                    check_first_operator(csp > cstack);
                    code = t1_hinter__rmoveto(h, *csp, 0);
                    goto move;
                } else
                    return_error(gs_error_invalidfont);
            case cx_vhcurveto:
                vertical = true;
                goto hvc;
            case cx_hvcurveto:
                vertical = false;
              hvc:for (ap = cstack; ap + 3 <= csp; vertical = !vertical, ap += 4) {
                    gs_fixed_point pt[2] = {{0, 0}, {0, 0}};
                    if (vertical) {
                        pt[0].y = ap[0];
                        pt[1].x = ap[3];
                        if (ap + 4 == csp)
                            pt[1].y = ap[4];
                    } else {
                        pt[0].x = ap[0];
                        if (ap + 4 == csp)
                            pt[1].x = ap[4];
                        pt[1].y = ap[3];
                    }
                    code = t1_hinter__rcurveto(h, pt[0].x, pt[0].y, ap[1], ap[2], pt[1].x, pt[1].y);
                    if (code < 0)
                        return code;
                }
                goto pp;

                        /***********************
                         * New Type 2 commands *
                         ***********************/

            case c2_blend:
                if (CS_CHECK_CSTACK_BOUNDS(csp, cstack))
                {
                    int n = fixed2int_var(*csp);
                    int num_values = csp - cstack;
                    gs_font_type1 *pfont = pcis->pfont;
                    int k = pfont->data.WeightVector.count;
                    int i, j;
                    cs_ptr base, deltas;

                    base = csp - 1 - num_values;
                    deltas = base + n - 1;
                    for (j = 0; j < n; j++, base++, deltas += k - 1)
                        for (i = 1; i < k; i++)
                            *base += (fixed)(deltas[i] *
                                pfont->data.WeightVector.values[i]);
                } else
                    return gs_note_error(gs_error_invalidfont);
                clear;
                continue;
            case c2_hstemhm:
              hstem: check_first_operator(!((csp - cstack) & 1));
                {
                    fixed x = 0;

                    for (ap = cstack; ap + 1 <= csp; x += ap[1], ap += 2) {
                            code = t1_hinter__hstem(h, x += ap[0], ap[1]);
                            if (code < 0)
                                return code;
                    }
                }
                pcis->num_hints += (csp + 1 - cstack) >> 1;
                clear;
                continue;
            case c2_hintmask:
                /*
                 * A hintmask at the beginning of the CharString is
                 * equivalent to vstemhm + hintmask.  For simplicity, we use
                 * this interpretation everywhere.
                 */
            case c2_cntrmask:
                if (CS_CHECK_CSTACK_BOUNDS(csp, cstack)) {
                    check_first_operator(!((csp - cstack) & 1));
                    type2_vstem(pcis, csp, cstack);
                }
                /*
                 * We should clear the stack here only if this is the
                 * initial mask operator that includes the implicit
                 * vstemhm, but currently this is too much trouble to
                 * detect.
                 */
                clear;
                {
                    byte mask[max_total_stem_hints / 8];
                    int i;

                    for (i = 0; i < pcis->num_hints; ++cip, i += 8) {
                        charstring_next(*cip, state, mask[i >> 3], encrypted);
                        if_debug1m('1', pfont->memory, " 0x%02x", mask[i >> 3]);
                    }
                    if_debug0m('1', pfont->memory, "\n");
                    ipsp->ip = cip;
                    ipsp->dstate = state;
                    if (c == c2_cntrmask) {
                        /****** NYI ******/
                    } else {	/* hintmask or equivalent */
                        if_debug0m('1', pfont->memory, "[1]hstem hints:\n");
                        if_debug0m('1', pfont->memory, "[1]vstem hints:\n");
                        code = t1_hinter__hint_mask(h, mask);
                        if (code < 0)
                            return code;
                    }
                }
                break;
            case c2_vstemhm:
              vstem:
                if (CS_CHECK_CSTACK_BOUNDS(csp, cstack)) {
                    check_first_operator(!((csp - cstack) & 1));
                    type2_vstem(pcis, csp, cstack);
                }else
                    return gs_note_error(gs_error_invalidfont);
                clear;
                continue;
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
                    ++ipsp;
                    cip = ipsp->cs_data.bits.data;
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