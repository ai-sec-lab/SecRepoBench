int
gs_type1_piece_codes(/*const*/ gs_font_type1 *pfont, /* lgtm[cpp/use-of-goto] */
                     const gs_glyph_data_t *pgd, gs_char *chars)
{
    gs_type1_data *const pdata = &pfont->data;
    /*
     * Decode the CharString looking for seac.  We have to process
     * callsubr, callothersubr, and return operators, but if we see
     * any other operators other than [h]sbw, pop, hint operators,
     * or endchar, we can return immediately.  We have to include
     * endchar because it is an (undocumented) equivalent for seac
     * in Type 2 CharStrings: see the cx_endchar case in
     * gs_type2_interpret in gstype2.c.
     *
     * It's really unfortunate that we have to duplicate so much parsing
     * code, but factoring out the parser from the interpreter would
     * involve more restructuring than we're prepared to do right now.
     */
    bool encrypted = pdata->lenIV >= 0;
    fixed cstack[ostack_size];
    fixed *csp;
    ip_state_t ipstack[ipstack_size + 1];
    ip_state_t *ipsp = &ipstack[0];
    const byte *cip, *end;
    crypt_state state;
    int c, hhints = 0, vhints = 0;
    int code, call_depth = 0;

    CLEAR_CSTACK(cstack, csp);
    cip = pgd->bits.data;
    end = pgd->bits.data + pgd->bits.size;
 call:
    state = crypt_charstring_seed;
    if (encrypted) {
        int skip = pdata->lenIV;

        /* Skip initial random bytes */
        for (; skip > 0; ++cip, --skip)
            decrypt_skip_next(*cip, state);
    }
 top:
    for (;;) {
        uint c0;

        if (cip >= end) {
             /* We used to treat buffer overrun as a simple invalid font, now we assume that
             * there is an implicit endchar/return.
             * Part of bug #693170 where the fonts are invalid (no endchar on some glyphs).
             */
            if (call_depth > 0)
                goto c_return;
            goto out;
        }
        c0 = *cip++;

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
                CS_CHECK_PUSH(csp, cstack);
                *++csp = int2fixed(lw);
            } else		/* not possible */
                return_error(gs_error_invalidfont);
            continue;
        }
#define cnext CLEAR_CSTACK(cstack, csp); goto top
        switch ((char_command) c) {
        default:
            cnext;
            break;
        case c2_shortint:
            {
                short sint = *cip++;
                sint = (sint << 8) + *cip++;
                CS_CHECK_PUSH(csp, cstack);
                *++csp = int2fixed(sint);
            }
            break;
        case c2_hstemhm:
            hhints += ((csp - cstack) + 1) / 2;
            cnext;
            break;
        case c2_vstemhm:
            vhints += ((csp - cstack) + 1) / 2;
            cnext;
            break;
        case c2_cntrmask:
            vhints += ((csp - cstack) + 1) / 2;
            cip += (vhints + hhints + 7) / 8;
            cnext;
            break;
        case c2_hintmask:
            {
                if (csp > cstack)
                    vhints += ((csp - cstack) + 1) / 2;
                cip += (vhints + hhints + 7) / 8;
                cnext;
            }
            break;
        case c2_callgsubr:
            call_depth++;
            if (csp < &(cstack[0])) {
                c = pdata->gsubrNumberBias;
            }
            else {
                c = fixed2int_var(*csp) + pdata->gsubrNumberBias;
            }
            CS_CHECK_IPSTACK(ipsp + 1, ipstack);
            code = pdata->procs.subr_data
                (pfont, c, true, &ipsp[1].cs_data);
            if (code < 0)
                return_error(code);
            if (csp >= &(cstack[0])) {
                --csp;
            }
            ipsp->ip = cip, ipsp->dstate = state, ipsp->ip_end = end;
            ++ipsp;
            cip = ipsp->cs_data.bits.data;
            end = ipsp->cs_data.bits.data + ipsp->cs_data.bits.size;
            goto call;
        case c_callsubr:
            call_depth++;
            if (csp < &(cstack[0])) {
                c = pdata->subroutineNumberBias;
            }
            else {
                c = fixed2int_var(*csp) + pdata->subroutineNumberBias;
            }
            CS_CHECK_IPSTACK(ipsp + 1, ipstack);
            code = pdata->procs.subr_data
                (pfont, c, false, &ipsp[1].cs_data);
            if (code < 0)
                return_error(code);
            if (csp >= &(cstack[0])) {
                --csp;
            }
            ipsp->ip = cip, ipsp->dstate = state, ipsp->ip_end = end;
            ++ipsp;
            cip = ipsp->cs_data.bits.data;
            end = ipsp->cs_data.bits.data + ipsp->cs_data.bits.size;
            goto call;
        case c_return:
c_return:
            if (call_depth == 0)
                return (gs_note_error(gs_error_invalidfont));
            else
                call_depth--;
            gs_glyph_data_free(&ipsp->cs_data, "gs_type1_piece_codes");
            CS_CHECK_IPSTACK(ipsp, ipstack);
            --ipsp;
            if (ipsp < ipstack)
                return (gs_note_error(gs_error_invalidfont));
            cip = ipsp->ip, state = ipsp->dstate, end = ipsp->ip_end;
            goto top;
        case cx_hstem:
            hhints += ((csp - cstack) + 1) / 2;
            cnext;
            break;
        case cx_vstem:
            vhints += ((csp - cstack) + 1) / 2;
            cnext;
            break;
        case c1_hsbw:
            cnext;
        case cx_endchar:
            if (csp < cstack + 3)
                goto out;	/* not seac */
        do_seac:
            /* This is the payoff for all this code! */
            if (CS_CHECK_CSTACK_BOUNDS(&csp[-1], cstack)) {
                chars[0] = fixed2int(csp[-1]);
                chars[1] = fixed2int(csp[0]);
                return 1;
            }
            else {
                return_error(gs_error_invalidfont);
            }
        case cx_escape:
            charstring_next(*cip, state, c, encrypted);
            ++cip;
            switch ((char1_extended_command) c) {
            default:
                goto out;
            case ce1_vstem3:
            case ce1_hstem3:
            case ce1_sbw:
                cnext;
            case ce1_pop:
                /*
                 * pop must do nothing, since it is used after
                 * subr# 1 3 callothersubr.
                 */
                goto top;
            case ce1_seac:
                goto do_seac;
            case ce1_callothersubr:
                if (CS_CHECK_CSTACK_BOUNDS(csp, cstack)) {
                    switch (fixed2int_var(*csp)) {
                    default:
                        goto out;
                    case 3:
                        if (csp >= &(cstack[1]))
                            csp -= 2;
                        goto top;
                    case 12:
                    case 13:
                    case 14:
                    case 15:
                    case 16:
                    case 17:
                    case 18:
                        cnext;
                    }
                }
                else {
                    return_error(gs_error_invalidfont);
                }
            }
        }
#undef cnext
    }
 out:
    return 0;
}